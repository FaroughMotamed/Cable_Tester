
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
*/

bool ads1220_init(SPI_HandleTypeDef *hspi);

#endif

