
#ifndef ADS1220_H
#define ADS1220_H
#include "main.h"
#include <stdbool.h>



/*
 Initialize the ADS1220.
 hspi points to the STM32 SPI peripheral used by the ADS1220.

 Returns:
      true  = initialization and register verification succeeded
      false = SPI communication or register verification failed

##############  ads1220_init sequence() ##############

                Receive SPI3 handle
                Store its address
                Deselect ADS1220
                Wait for power stabilization
                Reset ADS1220
                Write four configuration registers
                Read the registers back
                Compare expected and actual values
                Return true or false
*/

bool ads1220_init(SPI_HandleTypeDef *hspi);

typedef enum
{
    ADS1220_CABLE_SELECT = 0,
    ADS1220_RESISTOR_SELECT=1
} ads1220_input_type;


bool ads1220_select_input(ads1220_input_type input);

bool ads1220_start_conversion(void);

bool ads1220_wait_drdy(uint32_t timeout_ms);

bool ads1220_read_raw24bit_generate_32bit(int32_t *raw_code);



#endif