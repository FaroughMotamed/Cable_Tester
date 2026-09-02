
#include "ads1220.h"
#include <stdbool.h>
#include <stdint.h>

// ADS1220 SPI commands.
#define ADS1220_CMD_RESET           0x06U
#define ADS1220_CMD_START_SYNC      0x08U
#define ADS1220_CMD_POWERDOWN       0x02U
#define ADS1220_CMD_READDATA        0x10U
#define ADS1220_REGISTER_COUNT      4U
#define ADS1220_SPI_TIMEOUT_MS      100U
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

    ads1220_deselect(); // make the /CS high to deselect the ADS1220

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


// Read all four ADS1220 configuration registers.
static bool ads1220_read_registers(uint8_t registers[ADS1220_REGISTER_COUNT])
{
    uint8_t command = ADS1220_CMD_READ_ALL_REGISTERS;
    HAL_StatusTypeDef hal_status;

    if ((ads1220_spi == NULL) || (registers == NULL))
    {
        return false;
    }

    ads1220_select();

    
    // Tell the ADS1220 which registers I want to read.
    hal_status = HAL_SPI_Transmit(ads1220_spi, &command,  1U,  ADS1220_SPI_TIMEOUT_MS);

    if (hal_status == HAL_OK)
    {
        // Keep CS LOW and receive the four register values.
        hal_status = HAL_SPI_Receive(ads1220_spi, registers,  ADS1220_REGISTER_COUNT, ADS1220_SPI_TIMEOUT_MS);
    }

    ads1220_deselect();

    return (hal_status == HAL_OK);
}


/*
 ADS1220 configuration used by the cable tester.

 Register 0 = 0x01
     AINP = AIN0
     AINN = AIN1
     Gain = 1
     PGA bypassed

 Register 1 = 0x20
     Data rate = 45 SPS
     Normal operating mode
     Single-shot conversion mode
     Temperature sensor disabled
     Burnout current sources disabled

 Register 2 = 0x00
     Internal 2.048 V reference
     Low-side power switch open
     IDAC current sources disabled

 Register 3 = 0x00
     IDAC routing disabled
     Dedicated DRDY output mode
*/
static const uint8_t ads1220_configuration[ADS1220_REGISTER_COUNT] =
{
    0x01U,
    0x20U,
    0x00U,
    0x00U
};


// Compare values read from the ADS1220 with  values firmware attempted to write.
static bool ads1220_registers_match( const uint8_t expected[ADS1220_REGISTER_COUNT], const uint8_t actual[ADS1220_REGISTER_COUNT])
{
    for (uint8_t register_index = 0U; register_index < ADS1220_REGISTER_COUNT;  register_index++)
    {
        if (actual[register_index] != expected[register_index])
        {
            return false;
        }
    }

    return true;
}




bool ads1220_init(SPI_HandleTypeDef *hspi)
{
    uint8_t read_back[ADS1220_REGISTER_COUNT]; // array to keep the register data after read needed for comparison.

    if (hspi == NULL)
    {
        return false;
    }

    
    //Save the address of the SPI peripheral used by the ADS1220. For this project, hspi will normally be &hspi3.
    ads1220_spi = hspi;

    // deselected ADS1220 
    ads1220_deselect();


    // Allow  the sds1220 to settle for 2ms.
    HAL_Delay(2U);

    
    //Reset the ADS1220 through SPI.
    if (!ads1220_send_command(ADS1220_CMD_RESET))
    {
        return false;
    }

    
    // Wait for reset to complete. 50 microseconds + 32 × tCLK . it needs 2 ms.
    HAL_Delay(2U);

    /*
     * Write the four desired configuration registers.
     */
    if (!ads1220_write_registers(ads1220_configuration))
    {
        return false;
    }

    /*
     * Read the registers back from the ADS1220.
     */
    if (!ads1220_read_registers(read_back))
    {
        return false;
    }

    /*
     * Initialization succeeds only if the ADS1220 stored
     * exactly the configuration that we sent.
     */
    if (!ads1220_registers_match(
            ads1220_configuration,
            read_back))
    {
        return false;
    }

    return true;
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