

#include "cable_mux.h"
#include "main.h"

#define FIRST_CABLE_CONDUCTOR          1U
#define LAST_CABLE_CONDUCTOR           8U

#define ADG732_ADDRESS_BIT_0           0x01U
#define ADG732_ADDRESS_BIT_1           0x02U
#define ADG732_ADDRESS_BIT_2           0x04U
#define ADG732_ADDRESS_BIT_3           0x08U
#define ADG732_ADDRESS_BIT_4           0x10U

#define CABLE_MUX_SETTLING_DELAY_MS    2U


static void cable_mux_set_address(uint8_t mux_channel)
{
    GPIO_PinState pin_state;

    // Set ADG732 address bit A0.
    if ((mux_channel & ADG732_ADDRESS_BIT_0) != 0U)
    {
        pin_state = GPIO_PIN_SET;
    }
    else
    {
        pin_state = GPIO_PIN_RESET;
    }

    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, pin_state);

    // Set ADG732 address bit A1.
    if ((mux_channel & ADG732_ADDRESS_BIT_1) != 0U)
    {
        pin_state = GPIO_PIN_SET;
    }
    else
    {
        pin_state = GPIO_PIN_RESET;
    }

    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, pin_state);

    // Set ADG732 address bit A2.
    if ((mux_channel & ADG732_ADDRESS_BIT_2) != 0U)
    {
        pin_state = GPIO_PIN_SET;
    }
    else
    {
        pin_state = GPIO_PIN_RESET;
    }

    HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, pin_state);

    // Set ADG732 address bit A3.
    if ((mux_channel & ADG732_ADDRESS_BIT_3) != 0U)
    {
        pin_state = GPIO_PIN_SET;
    }
    else
    {
        pin_state = GPIO_PIN_RESET;
    }

    HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, pin_state);

    // Set ADG732 address bit A4.
    if ((mux_channel & ADG732_ADDRESS_BIT_4) != 0U)
    {
        pin_state = GPIO_PIN_SET;
    }
    else
    {
        pin_state = GPIO_PIN_RESET;
    }

    HAL_GPIO_WritePin(MUX_A4_GPIO_Port, MUX_A4_Pin, pin_state);
}


