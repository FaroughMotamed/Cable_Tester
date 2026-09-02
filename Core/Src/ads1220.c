
#include "ads1220.h"
#include <stdbool.h>
#include <stdint.h>

// ADS1220 SPI commands.
#define ADS1220_CMD_RESET           0x06U
#define ADS1220_CMD_START_SYNC      0x08U
#define ADS1220_CMD_POWERDOWN       0x02U
#define ADS1220_CMD_READDATA        0x10U

// U :unsinged integer

/*
 Read all four registers, starting at register 0.
 Read REG command format:
 0010 rr nn
 rr = starting register address
 nn = number of registers minus one
 
 Starting register rr = 0
 Number of registers = 4
 Therefore nn = 3

 Result: 0010 0011 = 0x23
 */
#define ADS1220_CMD_READ_ALL_REGISTERS  0x23U

/*
 Write all four registers, starting at register 0.
 Write REG command format:
 
 0100 rr nn
 
 Starting register rr= 0
 Number of registers = 4
 Therefore nn = 3

 Result: 0100 0011 = 0x43
 */

#define ADS1220_CMD_WRITE_ALL_REGISTERS 0x43U

#define ADS1220_REGISTER_COUNT  4U
#define ADS1220_SPI_TIMEOUT_MS  100U


// Pointer to the SPI peripheral supplied by ads1220_init().
// For this project, it will point to hspi3.
static SPI_HandleTypeDef *ads1220_spi = NULL;


// Select the ADS1220 for SPI communication.
// ADC_CS_N is active-low, so LOW selects the ADC.
static void ads1220_select(void)
{
    HAL_GPIO_WritePin(ADC_CS_N_GPIO_Port, ADC_CS_N_Pin, GPIO_PIN_RESET);
}


// End SPI communication with the ADS1220.
// HIGH deselects the ADC and releases its SPI interface.
static void ads1220_deselect(void)
{
    HAL_GPIO_WritePin(ADC_CS_N_GPIO_Port, ADC_CS_N_Pin, GPIO_PIN_SET);
}


//Send one command byte to the ADS1220.
static bool ads1220_send_command(uint8_t command)
{
    HAL_StatusTypeDef hal_status;

    //The SPI pointer must have been supplied by ads1220_init().
    if (ads1220_spi == NULL)
    {
        return false;
    }

    ads1220_select();

                               // SPI peripheral, Address of transmitted data, Number of bytes, Maximum waiting time
    hal_status = HAL_SPI_Transmit(ads1220_spi   , &command                   , 1U             , ADS1220_SPI_TIMEOUT_MS);

    ads1220_deselect();

    return (hal_status == HAL_OK);
}




// Write all four ADS1220 configuration registers.
static bool ads1220_write_registers(const uint8_t registers[ADS1220_REGISTER_COUNT])
{
    uint8_t transmit_data[5];
    HAL_StatusTypeDef hal_status;

    if ((ads1220_spi == NULL) || (registers == NULL))
    {
        return false;
    }

    
    // The first byte is the Write REG command.
    // The following four bytes are register values.
    transmit_data[0] = ADS1220_CMD_WRITE_ALL_REGISTERS;
    transmit_data[1] = registers[0];
    transmit_data[2] = registers[1];
    transmit_data[3] = registers[2];
    transmit_data[4] = registers[3];

    /*
    0x43:
    Write four configuration registers beginning at register 0

    0x01:
    Measure AIN0 − AIN1
    Gain 1
    PGA bypassed

    0x20:
    45 SPS
    Normal mode
    Single-shot
    Temperature sensor off
    Burnout current sources off

    0x00:
    Internal 2.048 V reference
    Low-side switch open
    IDAC current sources off

    0x00:
    IDAC routing disabled
    Normal DRDY operation
    */

    ads1220_select();

    hal_status = HAL_SPI_Transmit(ads1220_spi, transmit_data,  sizeof(transmit_data),  ADS1220_SPI_TIMEOUT_MS);

    ads1220_deselect();

    return (hal_status == HAL_OK);
}
































static int32_t ads1220_combine_bytes(const uint8_t data[3])
{
    uint32_t raw_code;

    /*
     * Place the three ADS1220 bytes in their correct positions:
     *
     * data[0] → bits 23–16
     * data[1] → bits 15–8
     * data[2] → bits 7–0
     */
    raw_code = ((uint32_t)data[0] << 16)
             | ((uint32_t)data[1] << 8)
             |  (uint32_t)data[2];



    int32_t signed_code;
    if ((raw_code & 0x00800000U) != 0U)
    {
        signed_code = (int32_t)(raw_code - 0x01000000U);
    }
    else
    {
        signed_code = (int32_t)raw_code;
    }

    return signed_code;
}