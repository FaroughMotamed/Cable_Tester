#include <main.h>
#include <stdbool.h>
#include <stdint.h>
#include "relays.h"
#include <stddef.h>


/*
BIT MASKING FOR RELAY OUTPUTS
============================

Each MCP23017 has 16 outputs:
    Port A: GPA0–GPA7 -> relay channels 1–8.
    Port B: GPB0–GPB7 -> relay channels 9–16.

For our active-high relay circuit:
    Bit 1 -> output HIGH -> relay ON.
    Bit 0 -> output LOW  -> relay OFF.

Channel numbers start at 1; bit numbers start at 0:
    bit_number = channel - 1

NUMBER FORMATS
--------------
Decimal, hexadecimal and binary can represent the same value:
    5 = 0x05 = binary 0101.

0x means hexadecimal. Each hexadecimal digit represents four bits:
    0 = 0000    1 = 0001    2 = 0010    3 = 0011
    C = 1100    D = 1101    E = 1110    F = 1111

The suffix U means unsigned.

SINGLE-CHANNEL MASKS
-------------------
Channel   Bit   Hex mask
   1       0    0x0001
   2       1    0x0002 = 0b 0000 0000 0000 0010
   3       2    0x0004 = 0b 0000 0000 0000 1000
  ...
  15      14    0x4000
  16      15    0x8000 = 0b 1000 0000 0000 0000

Example:
    0x0100 = binary 0000 0001 0000 0000.
    Only bit 8 is set, so only channel 9 is selected.
    Its numerical value is 256, not 9.

CREATE A MASK: LEFT SHIFT <<
---------------------------
Start with 1 and move it to the required bit position:

    uint16_t mask = (uint16_t)(1U << (channel - 1U));

Validate channel is 1–16 BEFORE calculating the mask.

Examples:
    1U << 2U -> 0x0004 -> channel 3.
    1U << 4U -> 0x0010 -> channel 5.

BITWISE OPERATORS
-----------------
OR  | : result bit is 1 if either input bit is 1.
AND & : result bit is 1 only if both input bits are 1.
NOT ~ : reverses every bit.

Use | and &, not logical operators || and &&.

REPLACE ALL OUTPUT STATES
-------------------------
    state = 0x0010U;  // Only channel 5 ON.
    state = 0x0000U;  // All channels OFF.
    state = 0xFFFFU;  // All channels ON.

SET SELECTED BITS, PRESERVING THE REST
-------------------------------------
    state |= mask;    // Same as: state = state | mask.

Example:
    state = 0x0004U;  // Channel 3 ON.
    state |= 0x0010U; // Add channel 5.
    // Result: 0x0014 -> channels 3 and 5 ON.

CLEAR SELECTED BITS, PRESERVING THE REST
--------------------------------------
    state &= (uint16_t)~mask;

Example:
    state = 0x0014U;             // Channels 3 and 5 ON.
    state &= (uint16_t)~0x0004U; // Turn channel 3 OFF.
    // Result: 0x0010 -> only channel 5 ON.

CHECK A SELECTED BIT
-------------------
    bool commanded_on = (state & mask) != 0U;

This checks the stored command, not physical relay contact movement.

CLEAR GROUPS OF BITS
-------------------
    state &= 0xFFF0U;
    // Mask: 1111 1111 1111 0000.
    // Clear bits 0–3: channels 1–4.
    // Preserve channels 5–16.

    state &= 0xF0FFU;
    // Mask: 1111 0000 1111 1111.
    // Clear bits 8–11: channels 9–12.
    // Preserve all other channels.

On shared board 010:
    Side A pins 17–20 -> channels 1–4.
    Side B pins 17–20 -> channels 9–12.

SPLIT THE 16-BIT PATTERN INTO TWO PORTS
-------------------------------------
    uint8_t port_a = (uint8_t)(state & 0x00FFU);
    uint8_t port_b = (uint8_t)(state >> 8U);

For state = 0x0100:
    port_a = 0x00 -> channels 1–8 OFF.
    port_b = 0x01 -> channel 9 ON.

SEND THE PATTERN TO THE HARDWARE
-------------------------------
Changing state only changes a variable in STM32 memory.
Call the driver to update the expander:

    bool success = MCP23017_WriteGPIO(board, state);

Board index -> actual 7-bit I2C address:
    0 -> 0x20
    1 -> 0x21
    2 -> 0x22

Example:
    MCP23017_WriteGPIO(1U, 0x0010U);
    // Board 001: only channel 5 ON.

This replaces all 16 output commands on THAT board.
Other boards are unaffected.
Always handle a false return: an I2C write failed, and outputs
may have changed only partially.

REGISTER AND DEVICE ADDRESSES
-----------------------------
OLATA = Port A output latch.
OLATB = Port B output latch.
IOCON = I/O configuration register.
BANK  = a bit in IOCON selecting the register-address layout.

With BANK = 0:
    OLATA address = 0x14.
    OLATB address = 0x15.

With BANK = 1:
    OLATA address = 0x0A.
    OLATB address = 0x1A.

Our driver requires BANK = 0 and pins configured as outputs.
Writing a latch stores the commanded levels until changed/reset.

STM32F4 HAL expects the 7-bit DEVICE address shifted left:
    device_address = (uint16_t)((0x20U + board) << 1U);

Example: actual address 0x21 -> HAL argument 0x42.
HAL manages the read/write bit.
Do NOT shift the register address or output data.

A write therefore specifies:
    Device address -> which expander.
    Register address -> which output latch.
    Data byte -> which eight outputs are HIGH/LOW.
*/


#include "main.h"
#include "relays.h"
#include <stdbool.h>
#include <stdint.h>

// Defined by CubeMX after enabling I2C1.
extern I2C_HandleTypeDef hi2c1;

#define MCP23017_BASE_ADDRESS  0x20U  //Starting I2C device address for the MCP23017.  7-bit device address with A2/A1/A0 = 000; add board index.

// Register addresses below assume IOCON.BANK = 0.
#define MCP23017_OLATA         0x14U  // Register address for the Port A output latch. Output latch A: commands levels for channels 1–8.
#define MCP23017_OLATB         0x15U  // Register address for the Port B output latch. Output latch B: commands levels for channels 9–16.
#define MCP23017_TIMEOUT_MS    100U   // Timeout parameter passed to HAL I2C calls, in milliseconds.

#define MCP23017_IODIRA        0x00U  // Register address for the direction of the eight Port B pins.
#define MCP23017_IODIRB        0x01U  // Register address for the direction of the eight Port B pins.
#define MCP23017_IOCON         0x0AU  // Register address for the chip's general configuration settings : BANK, sequential addressing, interrupts, etc.


/*
Examples:
MCP23017_WriteGPIO(0U, 0x0000U); // Board 000: all OFF.
MCP23017_WriteGPIO(1U, 0x0001U); // Board 001: channel 1 ON.
MCP23017_WriteGPIO(2U, 0x0100U); // Board 010: channel 9 ON.
*/
// function to write GPIOs of a certain I2C expandee board
bool MCP23017_WriteGPIO(uint8_t board, uint16_t outputs)
{
    uint16_t device_address;
    uint8_t port_a;
    uint8_t port_b;

    // This tester uses boards 000, 001 and 010.
    if (board > 2U)
    {
        return false;
    }

    /*
     Convert the board number to its I2C address:
         0 -> 0x20
         1 -> 0x21
         2 -> 0x22

     STM32 HAL expects the address shifted left by one bit.
    */
    device_address = (uint16_t)((MCP23017_BASE_ADDRESS + board) << 1U);

    // Bits 0–7 belong to Port A.
    port_a = (uint8_t)(outputs & 0x00FFU);

    // Bits 8–15 belong to Port B.
    port_b = (uint8_t)(outputs >> 8U);

    // Write the first eight relay states.
    /*
    HAL_I2C_Mem_Write(
    &hi2c1,                 // Use STM32 I2C1.
    device_address,         // Which expander: shifted address.
    MCP23017_OLATA,         // Which register: 0x14.
    I2C_MEMADD_SIZE_8BIT,   // Register address is one byte.
    &port_a,                // Address of the byte to send.
    1U,                     // Send one data byte.
    MCP23017_TIMEOUT_MS     // Timeout for this call.);
    
    */
    if (HAL_I2C_Mem_Write(&hi2c1,  // Use STM32 I2C1.
                          device_address,  // Which expander: shifted address.
                          MCP23017_OLATA,  // 0x14U
                          I2C_MEMADD_SIZE_8BIT,  // this valriable is 1, 
                          &port_a, // Address of the byte to send.
                          1U, // number of bytes to send
                          MCP23017_TIMEOUT_MS) // Timeout for this call
                          != HAL_OK)  
        return false;
    }

    // Write the remaining eight relay states.
    if (HAL_I2C_Mem_Write(&hi2c1,
                          device_address,
                          MCP23017_OLATB,
                          I2C_MEMADD_SIZE_8BIT,
                          &port_b,
                          1U,
                          MCP23017_TIMEOUT_MS) != HAL_OK)
    {
        return false;
    }

    return true;
}

/*
Requiremnts before using it

Two requirements before using it:
- CubeMX must initialize I2C1 on PB8/PB9.
- Relay_Init() must establish BANK = 0, preload the latches with zero, and configure the expander pins as outputs.
*/



bool MCP23017_ReadGPIO(uint8_t board, uint16_t *outputs)
{
    uint16_t device_address;
    uint8_t port_a;
    uint8_t port_b;

    // Validate the output pointer.
    if (outputs == NULL)
    {
        return false;
    }

    *outputs = 0U;

    // Only boards 000, 001 and 010 are used.
    if (board > 2U)
    {
        return false;
    }

    device_address = (uint16_t)((MCP23017_BASE_ADDRESS + board) << 1U);

    // Read the commanded states for channels 1–8.
    if (HAL_I2C_Mem_Read(&hi2c1,
                         device_address,
                         MCP23017_OLATA,
                         I2C_MEMADD_SIZE_8BIT,
                         &port_a,
                         1U,
                         MCP23017_TIMEOUT_MS) != HAL_OK)
    {
        return false;
    }

    // Read the commanded states for channels 9–16.
    if (HAL_I2C_Mem_Read(&hi2c1,
                         device_address,
                         MCP23017_OLATB,
                         I2C_MEMADD_SIZE_8BIT,
                         &port_b,
                         1U,
                         MCP23017_TIMEOUT_MS) != HAL_OK)
    {
        return false;
    }

    // Place Port B in bits 8–15 and Port A in bits 0–7.
    *outputs = (uint16_t)(  ((uint16_t)port_b << 8U) | (uint16_t)port_a  );

    return true;
}

// write a single pin in a board.
// Example:
// Shared board 010: turn channel 9 ON.
// write_result = MCP23017_WritePin(2U, 8U, true);
bool MCP23017_WritePin(uint8_t board, uint8_t pin, bool on)
{
    uint16_t state;
    uint16_t mask;
    bool write_result;

    // Validate board number and output bit index.
    if ((board > 2U) || (pin > 15U))
    {
        return false;
    }

    // Read the existing output commands.
    if (!MCP23017_ReadGPIO(board, &state))
    {
        return false;
    }

    // Create a mask with only the requested bit set.
    mask = (uint16_t)(1U << pin);

    if (on)
    {
        // Set this bit; preserve all other bits.
        state |= mask;
    }
    else
    {
        // Clear this bit; preserve all other bits.
        state &= (uint16_t)~mask;
    }

    // Send the updated commands to the expander.
    write_result= MCP23017_WriteGPIO(board, state);

    return write_result;
}


bool Relay_AllOff(void)
{
    bool success = true;

    // Board 000: Side-A cable pins 1–16.
    if (!MCP23017_WriteGPIO(0U, 0x0000U))
    {
        success = false;
    }

    // Board 001: Side-B cable pins 1–16.
    if (!MCP23017_WriteGPIO(1U, 0x0000U))
    {
        success = false;
    }

    // Board 010: remaining Side-A and Side-B pins.
    if (!MCP23017_WriteGPIO(2U, 0x0000U))
    {
        success = false;
    }

    return success;
}

bool Relay_SelectSideA(uint8_t pin)
{
    // Cable pin numbers are 1–20.
    // Reject invalid numbers before changing any outputs.
    if ((pin < 1U) || (pin > 20U))
    {
        return false;
    }

    // Release the previous Side-A relay.
    // Do not activate another relay if disconnection fails.
    if (!Relay_DisconnectSideA())
    {
        return false;
    }

    if (pin <= 16U)
    {
        // Board 000: cable pins 1–16 use bits 0–15.
        return MCP23017_WritePin(0U, (uint8_t)(pin - 1U), true);
    }

    // Shared board 010: cable pins 17–20 use bits 0–3 for side A
    // Side B's output commands are preserved.
    return MCP23017_WritePin(2U, (uint8_t)(pin - 17U), true);
}


bool Relay_SelectSideB(uint8_t pin)
{
    // Cable pin numbers are 1–20.
    // Reject invalid numbers before changing any outputs.
    if ((pin < 1U) || (pin > 20U))
    {
        return false;
    }

    // Release the previous Side-B relay.
    // Stop if the disconnection commands fail.
    if (!Relay_DisconnectSideB())
    {
        return false;
    }

    if (pin <= 16U)
    {
        // Board 001: cable pins 1–16 use bits 0–15.
        return MCP23017_WritePin(1U, (uint8_t)(pin - 1U), true);
    }

    // Shared board 010: cable pins 17–20 use bits 8-11 for side B.
    // Side A's output commands are preserved.
    return MCP23017_WritePin(2U, (uint8_t)(pin - 9U),  true);
}

bool Relay_DisconnectSideA(void)
{
    uint16_t shared_state;
    bool success = true;

    // Board 000 controls Side-A cable pins 1–16.
    // Command all its relays OFF.
    if (!MCP23017_WriteGPIO(0U, 0x0000U))
    {
        success = false;
    }

    /*
     Board 010 is shared:
         Bits 0–3  -> Side-A cable pins 17–20.
         Bits 8–11 -> Side-B cable pins 17–20.

     Read the existing commands so Side B can be preserved.
    */
    if (!MCP23017_ReadGPIO(2U, &shared_state))
    {
        return false;
    }

    // Clear only Side A's four bits.
    shared_state &= 0xFFF0U;

    if (!MCP23017_WriteGPIO(2U, shared_state))
    {
        success = false;
    }

    return success;
}

bool Relay_DisconnectSideB(void)
{
    uint16_t shared_state;
    bool success = true;

    // Board 001 controls Side-B cable pins 1–16.
    // Command all its relays OFF.
    if (!MCP23017_WriteGPIO(1U, 0x0000U))
    {
        success = false;
    }

    // Read shared board 010 to preserve Side A's commands.
    if (!MCP23017_ReadGPIO(2U, &shared_state))
    {
        return false;
    }

    /*
     Clear bits 8–11: shared-board channels 9–12.
     These control Side-B cable pins 17–20.
     Preserve all other bits.
    */
    shared_state &= 0xF0FFU;

    if (!MCP23017_WriteGPIO(2U, shared_state))
    {
        success = false;
    }

    return success;
}


bool Relay_Init(void)
{
    uint8_t board;
    uint8_t register_index;
    uint8_t value;
    uint8_t readback;
    uint16_t device_address;

    /*
     Register order matters:
         1. Configure IOCON.
         2. Preload both output latches with OFF values.
         3. Enable the pins as outputs.
    */
    const uint8_t registers[] =
    {
        MCP23017_IOCON,
        MCP23017_OLATA,
        MCP23017_OLATB,
        MCP23017_IODIRA,
        MCP23017_IODIRB
    };

    // Initialize boards 000, 001 and 010.
    for (board = 0U; board < 3U; board++)
    {
        device_address = (uint16_t)((MCP23017_BASE_ADDRESS + board) << 1U);

        // Check whether this expander acknowledges its address.
        if (HAL_I2C_IsDeviceReady(&hi2c1, device_address, 3U, MCP23017_TIMEOUT_MS) != HAL_OK)
        {
            return false;
        }

        /*
         Establish BANK = 0 even if the chip was left in BANK = 1.

         At address 0x05:
             BANK = 1: this is IOCON; zero changes BANK to 0.
             BANK = 0: this is GPINTENB; zero disables Port-B input interrupts, which we do not use.

         This write does not change the relay output latches.
        */
        value = 0x00U;

        if (HAL_I2C_Mem_Write(&hi2c1,
                              device_address,
                              0x05U,
                              I2C_MEMADD_SIZE_8BIT,
                              &value,
                              1U,
                              MCP23017_TIMEOUT_MS) != HAL_OK)
        {
            return false;
        }

        // Write and verify each initialization register.
        for (register_index = 0U; register_index < sizeof(registers); register_index++)
        {
            value = 0x00U;

            if (HAL_I2C_Mem_Write(&hi2c1,
                                  device_address,
                                  registers[register_index],
                                  I2C_MEMADD_SIZE_8BIT,
                                  &value,
                                  1U,
                                  MCP23017_TIMEOUT_MS) != HAL_OK)
            {
                return false;
            }

            if (HAL_I2C_Mem_Read(&hi2c1,
                                 device_address,
                                 registers[register_index],
                                 I2C_MEMADD_SIZE_8BIT,
                                 &readback,
                                 1U,
                                 MCP23017_TIMEOUT_MS) != HAL_OK)
            {
                return false;
            }

            if (readback != value)
            {
                return false;
            }
        }
    }

    // Allow previously energized relays time to release.
    HAL_Delay(50U);

    return true;
}



#define RELAY_SETTLE_TIME_MS  50U

bool Relay_SelectPath(uint8_t a_pin, uint8_t b_pin)
{
    uint16_t board_a_state;
    uint16_t shared_state;
    uint16_t expected_board_a = 0U;
    uint16_t expected_shared_a = 0U;
    bool side_a_matches;

    // Reject invalid cable-pin numbers without changing outputs.
    if ((a_pin < 1U) || (a_pin > 20U) ||
        (b_pin < 1U) || (b_pin > 20U))
    {
        return false;
    }

    // Read the existing Side-A output commands.
    if (!MCP23017_ReadGPIO(0U, &board_a_state))
    {
        (void)Relay_AllOff();
        return false;
    }

    if (!MCP23017_ReadGPIO(2U, &shared_state))
    {
        (void)Relay_AllOff();
        return false;
    }

    // Calculate the required Side-A output pattern.
    if (a_pin <= 16U)
    {
        expected_board_a = (uint16_t)(1U << (a_pin - 1U));
    }
    else
    {
        expected_shared_a = (uint16_t)(1U << (a_pin - 17U));
    }

    // Only bits 0–3 of the shared board belong to Side A.
    side_a_matches = (board_a_state == expected_board_a) && ((shared_state & 0x000FU) == expected_shared_a);

    // Keep Side A energized when it is already selected correctly.
    if (!side_a_matches)
    {
        if (!Relay_SelectSideA(a_pin))
        {
            (void)Relay_AllOff();
            return false;
        }
    }

    // Release the previous B relay and activate the requested one.
    if (!Relay_SelectSideB(b_pin))
    {
        (void)Relay_AllOff();
        return false;
    }

    // Give release and activation one shared settling period.
    HAL_Delay(RELAY_SETTLE_TIME_MS);

    return true;
}







