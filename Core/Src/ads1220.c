
#include "ads1220.h"
#include <stdbool.h>
#include <stdint.h>

/*=================================================== Start of comment section =========================================================*/
//========================== Communication method for ADS1220 (24 bit analogue digital converter component (ADC)) =======================

/*
ADS1220 reads the voltage difference across the cable threads and across the 200 Ohm precision resistor.
Voltage readings are used to caclulate the current the goes through the cable thread and then calculate the resistance of the cable thread.

STM32 commnicates to the ADS1220 through SPI.
Note: commands required to write and read registers are different from the commands required to read voltage conversion results.

Sequence of communication:
1. First we write into registers to determine what ADC channels (AIN 0-1 , AIN 2-3) to use and what the reading speed is.
2. Then we read the registers back to make sure writing the registers was successful.
3. Then we send commands to read the conversion results.

Command structure to write and read registers:
5 bytes are needed to send a complete regiater write or read command. (fewere registers can be used if the rest are already set)
The first byte determines whether the command is write or read  and how many registers to write/read starting from what register.
The rest of the 4 bytes determine the ADC channel (AIN 0-1 , AIN 2-3), speed of reading , etc.
*/

/*
Example register configuration used by the cable tester.

//  calculate the command to set up the write process into all registers starting at register 0. Then  determine the value of each registor 0 to 4.

    Write all four registers, starting at register 0.
    Write REG command format:
 
    0100 rrnn
    
    Starting register rr= 0
    Number of registers = 4
    Therefore nn = 3  ----> rrnn = 0x03 = 0011

    Result: 0100 0011 = 0x43  ---> define ADS1220_CMD_WRITE_ALL_REGISTERS 0x43U

    //
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

/*
Example command to select channel AIN0-AIN1: 
command    (1st byte ) : 0x40 → write one register starting at register 0
Register 0 (2nd byte ) : 0x01 → select AIN0 − AIN1, gain 1, PGA bypassed
*/


/*
The sequence to read the voltage conversion from each input pair (AIN 0-1, AIN 2-3)
0. Send the required write command to determine what register/registers you want to write into.
1. Write the input selection into configuration register 0.
2. Send START/SYNC.
3. Wait for DRDY to become low.
4. Send READYDATA command.
5. Read three conversion bytes.
6. If the result is wrong? Then  read again.
*/

/*
The complete SPI sequence for the AIN 0-1 measurment is:

CS LOW      // select the Ads1220
Send 0x40   // send the write register command (through SPI)
Send 0x01   // select channel AIN 0-1  0x01,  Select AIN2 − AIN3   0x31   (through SPI)
CS HIGH     // de-select the ADC

CS LOW      // select the Ads1220
Send 0x08   // Send START command (SPI)
CS HIGH     // de-select the ADC

Wait until DRDY LOW

CS LOW       // select the Ads1220
Send 0x10    // send READYDATA command (SPI)
Read byte 2  // READ 3 bytes
Read byte 1
Read byte 0
CS HIGH      // de-select the ADC
*/


/* ===============  Example how to calculate command and register values for reading channel AIN 0-1 =================
Calculate values for AIN 0-1 one time measurment:

## Write command for a single register starting from register 0: 0x40U

## Calculate value for register 0
MUX  = 0000 → AIN0 − AIN1
GAIN = 000  → gain 1
PGA_BYPASS = 1
Register 0 = 0000 0001 = 0x01

// Example code for SPI command structure

uint8_t select_ain0_ain1[2] = {  0x40U,  0x01U  };

ads1220_select();
                   SPI peripheral,     Address of transmitted data,    Number of bytes,    Maximum waiting time
HAL_SPI_Transmit(  ads1220_spi   ,     select_ain0_ain1           ,    2U             ,    ADS1220_SPI_TIMEOUT_MS);
*/

/*===================================== End of comment section ===============================================*/

//  command value to write all registers starting at register 0
#define ADS1220_CMD_WRITE_ALL_REGISTERS 0x43U
/*
 Write all four registers, starting at register 0.
 Write REG command format:
 
 0100 rrnn
 
 Starting register rr= 0
 Number of registers = 4
 Therefore nn = 3
 rrnn = 03 = 0x0011

 Result: 0100 0011 = 0x43
*/

//  command value to read all registers starting at register 0
#define ADS1220_CMD_READ_ALL_REGISTERS  0x23U
/*
 Read all four registers, starting at register 0.
 Read REG command format:
 0010 rrnn
 rr = starting register address
 nn = number of registers minus one
 
 Starting register rr = 0
 Number of registers = 4
 Therefore nn = 3 
 rrnn = 03 = 0x0011

 Result: 0010 0011 = 0x23
 */


// ADS1220 SPI commands.
#define ADS1220_CMD_RESET           0x06U
#define ADS1220_CMD_START_SYNC      0x08U
#define ADS1220_CMD_POWERDOWN       0x02U
#define ADS1220_CMD_READDATA        0x10U
#define ADS1220_REGISTER_COUNT      4U
#define ADS1220_SPI_TIMEOUT_MS      100U

#define ADS1220_CMD_WRITE_REGISTER  0x40U
#define ADS1220_REG_CONFIG_0        0x00U
#define ADS1220_MUX_MASK            0xF0U
#define ADS1220_MUX_AIN0_AIN1       0x00U
#define ADS1220_MUX_AIN2_AIN3       0x30U



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


/*   ads1220_write_register function description:
Write one value into one ADS1220 configuration register.
 register_address:
     Configuration register address from 0 to 3.
 register_value:
     Value to store in that configuration register.
 Returns:
     true  when the SPI transmission succeeds.
     false when an argument is invalid or SPI fails.
*/
bool ads1220_write_register( uint8_t register_address, uint8_t register_value)
{
    uint8_t transmit_data[2];
    HAL_StatusTypeDef hal_status;

    if (ads1220_spi == NULL)
    {
        return false;
    }

    
    // The ADS1220 has four configuration registers:
    // register 0, 1, 2 and 3.
    if (register_address >= ADS1220_REGISTER_COUNT)
    {
        return false;
    }

    /*
     Build the write register command:
     
     Bits 7:4 = 0100: write-register command
     Bits 3:2 = starting register address
     Bits 1:0 = number of registers minus one
     
     This function writes  one register, so bits 1:0 remain 00.
     */
                                                       // first extract the last two bits from the register_address variable. --> register_address & 0x03U
                                                       // since only one register is written, the last two bits remain zero uisng --> <<2U.
    transmit_data[0] =  ADS1220_CMD_WRITE_REGISTER |  ((register_address & 0x03U) << 2U);

    // The second SPI byte is the value that will be stored in the selected register.
    transmit_data[1] = register_value;

    // Pull ADC_CS_N low to select the ADS1220.
    ads1220_select();


    // Send both bytes:
    // transmit_data[0] = command and register address
    // transmit_data[1] = new register value
    hal_status = HAL_SPI_Transmit( ads1220_spi,  transmit_data,  2U, ADS1220_SPI_TIMEOUT_MS);

 
    // release chip select after the transaction.
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
    uint8_t read_back[ADS1220_REGISTER_COUNT]; // read_back is an array to keep the register data after read needed for comparison.

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


    // sent and read configuration must match
    if (!ads1220_registers_match( ads1220_configuration, read_back))
    {
        return false;
    }

    return true;
}



bool ads1220_select_input(ads1220_input_type input){



}






// bool ads1220_select_input(...);
// bool ads1220_start_conversion(void);
// bool ads1220_wait_drdy(uint32_t timeout_ms);
// bool ads1220_read_raw(int32_t *raw_code);
// float ads1220_code_to_voltage(int32_t raw_code);





















































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