

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

#define ADG732_FIRST_CHANNEL    1U
#define ADG732_LAST_CHANNEL     32U


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


static void cable_mux_latch_address(void)
{
    /*
    Select all four ADG732 devices.
    The chip-select signals are active low, so writing RESET selects each MUX.
    */
    HAL_GPIO_WritePin(MUX1_CS_N_GPIO_Port, MUX1_CS_N_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX2_CS_N_GPIO_Port, MUX2_CS_N_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX3_CS_N_GPIO_Port, MUX3_CS_N_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX4_CS_N_GPIO_Port, MUX4_CS_N_Pin, GPIO_PIN_RESET);

    
    //Bring WR low while the address on A0 through A4 is stable.
    HAL_GPIO_WritePin(MUX_WR_N_GPIO_Port, MUX_WR_N_Pin, GPIO_PIN_RESET);

    // Provide a short address setup and WR pulse delay.
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();


    //The rising edge of WR stores the channel address inside every selected ADG732.
    HAL_GPIO_WritePin(MUX_WR_N_GPIO_Port,  MUX_WR_N_Pin, GPIO_PIN_SET);

    // Allow a short hold time after the WR rising edge.
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    /*
    Deselect all four MUXes after the address has been stored.
    The selected analog channel remains latched.*/
    HAL_GPIO_WritePin(MUX1_CS_N_GPIO_Port, MUX1_CS_N_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MUX2_CS_N_GPIO_Port, MUX2_CS_N_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MUX3_CS_N_GPIO_Port, MUX3_CS_N_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MUX4_CS_N_GPIO_Port, MUX4_CS_N_Pin, GPIO_PIN_SET);
}


static void cable_mux_enable_all(void)
{
    // Enable MUX 1.
    HAL_GPIO_WritePin(MUX1_EN_N_GPIO_Port, MUX1_EN_N_Pin, GPIO_PIN_RESET);

    // Enable MUX 2.
    HAL_GPIO_WritePin(MUX2_EN_N_GPIO_Port, MUX2_EN_N_Pin, GPIO_PIN_RESET);

    // Enable MUX 3.
    HAL_GPIO_WritePin(MUX3_EN_N_GPIO_Port, MUX3_EN_N_Pin, GPIO_PIN_RESET);

    // Enable MUX 4.
    HAL_GPIO_WritePin(MUX4_EN_N_GPIO_Port,  MUX4_EN_N_Pin, GPIO_PIN_RESET);
}

void cable_mux_disable_all(void)
{
    // EN_N high disconnects all channels in MUX 1.
    HAL_GPIO_WritePin(MUX1_EN_N_GPIO_Port, MUX1_EN_N_Pin, GPIO_PIN_SET);

    // EN_N high disconnects all channels in MUX 2.
    HAL_GPIO_WritePin(MUX2_EN_N_GPIO_Port, MUX2_EN_N_Pin, GPIO_PIN_SET);

    // EN_N high disconnects all channels in MUX 3.
    HAL_GPIO_WritePin(MUX3_EN_N_GPIO_Port, MUX3_EN_N_Pin, GPIO_PIN_SET);

    // EN_N high disconnects all channels in MUX 4.
    HAL_GPIO_WritePin(MUX4_EN_N_GPIO_Port,  MUX4_EN_N_Pin, GPIO_PIN_SET);
}

// All 4 Muxes select the conductor and are enabled. 
bool cable_mux_select_conductor(uint8_t conductor_number)
{
    uint8_t mux_channel;

    // Disconnect the analog paths before changing channels.
    cable_mux_disable_all();

    // Convert conductor 1-8 to ADG732 address 0-7.
    mux_channel = conductor_number - 1U;

    // Put the new address on A0-A4.
    cable_mux_set_address(mux_channel);

    // Store that address in all four MUXes.
    cable_mux_latch_address();

    // Connect the selected channels.
    cable_mux_enable_all();

    // Allow the complete analog circuit to settle.
    HAL_Delay(CABLE_MUX_SETTLING_DELAY_MS);

    return true;
}




#define FIRST_CABLE_CONDUCTOR           1U
#define LAST_CABLE_CONDUCTOR            20U
#define CABLE_MUX_SETTLING_DELAY_MS     2U

bool cable_mux_select_path( uint8_t end_a_conductor, uint8_t end_b_conductor)
{
    // Validate the selected conductor at end A.
    if ((end_a_conductor < FIRST_CABLE_CONDUCTOR) ||  (end_a_conductor > LAST_CABLE_CONDUCTOR))
    {
        cable_mux_disable_all();
        return false;
    }

    // Validate the selected conductor at end B.
    if ((end_b_conductor < FIRST_CABLE_CONDUCTOR) || (end_b_conductor > LAST_CABLE_CONDUCTOR))
    {
        cable_mux_disable_all();
        return false;
    }

    // Disconnect every analog path before changing addresses.
    cable_mux_disable_all();

    // Program both MUXes associated with cable end A.
    if (!cable_mux_set_channel(CABLE_MUX_END_A_FORCE, end_a_conductor))
    {
        cable_mux_disable_all();
        return false;
    }

    if (!cable_mux_set_channel(CABLE_MUX_END_A_SENSE, end_a_conductor))
    {
        cable_mux_disable_all();
        return false;
    }

    // Program both MUXes associated with cable end B.
    if (!cable_mux_set_channel(CABLE_MUX_END_B_FORCE, end_b_conductor))
    {
        cable_mux_disable_all();
        return false;
    }

    if (!cable_mux_set_channel(CABLE_MUX_END_B_SENSE, end_b_conductor))
    {
        cable_mux_disable_all();
        return false;
    }

    // Enable the two paths at cable end A.
    cable_mux_enable(CABLE_MUX_END_A_FORCE);
    cable_mux_enable(CABLE_MUX_END_A_SENSE);

    // Enable the two paths at cable end B.
    cable_mux_enable(CABLE_MUX_END_B_FORCE);
    cable_mux_enable(CABLE_MUX_END_B_SENSE);

    // Allow the complete analog path to settle.
    HAL_Delay(CABLE_MUX_SETTLING_DELAY_MS);

    return true;
}



static void cable_mux_latch_address(cable_mux_id_t mux)
{
    // Begin with every MUX control interface deselected.
    cable_mux_deselect_all();

    // Select only the MUX that should store the address.
    switch (mux)
    {
        case CABLE_MUX_END_A_FORCE:

            HAL_GPIO_WritePin(MUX1_CS_N_GPIO_Port,
                              MUX1_CS_N_Pin,
                              GPIO_PIN_RESET);
            break;

        case CABLE_MUX_END_A_SENSE:

            HAL_GPIO_WritePin(MUX2_CS_N_GPIO_Port,
                              MUX2_CS_N_Pin,
                              GPIO_PIN_RESET);
            break;

        case CABLE_MUX_END_B_FORCE:

            HAL_GPIO_WritePin(MUX3_CS_N_GPIO_Port,
                              MUX3_CS_N_Pin,
                              GPIO_PIN_RESET);
            break;

        case CABLE_MUX_END_B_SENSE:

            HAL_GPIO_WritePin(MUX4_CS_N_GPIO_Port,
                              MUX4_CS_N_Pin,
                              GPIO_PIN_RESET);
            break;

        default:

            // The caller validates the MUX before reaching here.
            return;
    }

    // Begin the write operation.
    HAL_GPIO_WritePin(MUX_WR_N_GPIO_Port,
                      MUX_WR_N_Pin,
                      GPIO_PIN_RESET);

    // Provide a short setup and write-pulse time.
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    // The WR rising edge stores the address in the selected MUX.
    HAL_GPIO_WritePin(MUX_WR_N_GPIO_Port,
                      MUX_WR_N_Pin,
                      GPIO_PIN_SET);

    // Provide a short address hold time.
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    // End the write by returning every CS signal high.
    cable_mux_deselect_all();
}


