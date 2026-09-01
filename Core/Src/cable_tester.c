#include "main.h"
#include "cable_tester.h"


static void CableTester_EnterSafeState(void)
{
    
    //Disable all four Muxes
    HAL_GPIO_WritePin(MUX_FA_EN_N_GPIO_Port,  MUX_FA_EN_N_Pin,  GPIO_PIN_SET);

    /* TODO: Disable Force-B MUX. */
    /* TODO: Disable Sense-A MUX. */
    /* TODO: Disable Sense-B MUX. */


    /*
     * Step 2:
     * Deselect all four MUX control interfaces.
     *
     * Because /CS is active LOW:
     * HIGH = not selected
     */

    /* TODO: Set all four MUX /CS outputs HIGH. */


    /*
     * Step 3:
     * Ensure no MUX address write is happening.
     *
     * /WR HIGH = inactive
     */

    /* TODO: Set MUX_WR_N HIGH. */


    /*
     * Step 4:
     * Establish a known MUX address.
     *
     * A4 A3 A2 A1 A0 = 0 0 0 0 0
     *
     * This represents logical channel S1, but it is harmless
     * because every MUX remains disabled.
     */

    /* TODO: Set MUX_A0 through MUX_A4 LOW. */


    /*
     * Step 5:
     * Deselect the ADS1220.
     */

    /* TODO: Set ADC_CS_N HIGH. */


    /*
     * Step 6:
     * Put display control outputs in an inactive state.
     */

    /* TODO: Set LCD_CS_N HIGH. */
    /* TODO: Set FLASH_CS_N HIGH. */
    /* TODO: Set LCD_RESET_N HIGH. */
    /* TODO: Choose an initial state for LCD_DCX. */


    /*
     * Step 7:
     * Turn off all indicators.
     */

    /* TODO: Set LED_FAIL LOW. */
    /* TODO: Set LED_PASS LOW. */
    /* TODO: Set LED_STATUS LOW. */
    /* TODO: Set BUZZER_EN LOW. */
}