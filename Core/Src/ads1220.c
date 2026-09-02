
#include "ads1220.h"

// ADS1220 SPI commands.
#define ADS1220_CMD_RESET       0x06U
#define ADS1220_CMD_START_SYNC  0x08U
#define ADS1220_CMD_POWERDOWN   0x02U
#define ADS1220_CMD_RDATA       0x10U

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

















