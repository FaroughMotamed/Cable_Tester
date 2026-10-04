#include <main.h>
#include <stdbool.h>
#include <stdint.h>



void Relay_AllOff(void)
{
    /*
    Board 000:  0000 0000 0000 0000
    Board 001:  0000 0000 0000 0000
    Board 010:  0000 0000 0000 0000
    */

    // Board 000 - Side A
    MCP23017_WriteGPIO(0x00, 0x0000);

    // Board 001 - Side B
    MCP23017_WriteGPIO(0x01, 0x0000);

    // Board 010 - Shared A/B
    MCP23017_WriteGPIO(0x02, 0x0000);
}

void Relay_SelectSideA(uint8_t pin)
{
    // First disconnect all Side-A relays
    Relay_DisconnectSideA();

    /*
     Select the requested Side-A cable pin.
     Pins handled by relay board 000 come first.
     Remaining Side-A pins use channels 1-4 of shared relay board 010.
     */

    if (pin >= 1 && pin <= 16)
    {
        /* Board 000 */
        MCP23017_WritePin(0x00, pin - 1, 1);
    }
    else if (pin >= 17 && pin <= 20)
    {
        /* Shared board 010, channels 1-4 */
        MCP23017_WritePin(0x02, pin - 17, 1);
    }
}



void Relay_DisconnectSideA(void)
{
    // Board 000 belongs entirely to Side A
    MCP23017_WriteGPIO(0x00, 0x0000);

    /*
     Shared board 010:
     Clear channels 1-4 only.
     Preserve all other channels
     */
    uint16_t state = MCP23017_ReadGPIO(0x02);

    state = state & (0xFFF0);

    MCP23017_WriteGPIO(0x02, state);
}


void Relay_DisconnectSideB(void)
{
    uint16_t state;

    // Turn OFF all relays on Side-B board 001 
    MCP23017_WriteGPIO(0x01, 0x0000);

    /*
     Shared board 010:
     Clear CH9-CH12 while preserving all other channels.
     Mask for bits 9 to 12: 0x0F00 =  0000 1111 0000 0000
     we want to clear them:  0xF0FF = 1111 0000 1111 1111
     */
    state = MCP23017_ReadGPIO(0x02);

    state = state & 0xF0FF;

    MCP23017_WriteGPIO(0x02, state);
}


void Relay_SelectSideB(uint8_t pin)
{
    // Reject invalid pins before changing the current path.
    if ((pin < 1U) || (pin > 20U))
    {
        return;
    }

    // Release the previous Side-B relay.
    // Side A remains selected.
    Relay_DisconnectSideB();

    if (pin <= 16U)
    {
        // Board 001: cable pins 1–16 use bits 0–15.
        MCP23017_WritePin(0x01U, pin - 1U, 1U);
    }
    else
    {
        // Board 010: cable pins 17–20 use bits 8–11.
        MCP23017_WritePin(0x02U, pin - 9U, 1U);
    }
}





