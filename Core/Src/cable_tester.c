
#include "main.h"
#include "cable_tester.h"


static void enter_safe_state(void)
{
    // Different paramters used for disabling the muxes are defined in the main.h

    /*=====================================================================================================*/
    // *****************************************  Disable 4 MUXes ******************************************

    // Set enable pins to high
    // 4 MUX /EN pins are set to HIGH (pins are active low)
    HAL_GPIO_WritePin(MUX_FA_EN_N_GPIO_Port,  MUX_FA_EN_N_Pin,  GPIO_PIN_SET); // Disable Force-A MUX
    HAL_GPIO_WritePin(MUX_FB_EN_N_GPIO_Port,  MUX_FB_EN_N_Pin,  GPIO_PIN_SET); // Disable Force-B MUX
    HAL_GPIO_WritePin(MUX_SA_EN_N_GPIO_Port,  MUX_SA_EN_N_Pin,  GPIO_PIN_SET); // Disable Sense-A MUX
    HAL_GPIO_WritePin(MUX_SB_EN_N_GPIO_Port,  MUX_SA_EN_N_Pin,  GPIO_PIN_SET); // Disable Sense-B MUX

    // Deselect all four individual MUX control interfaces.
    // 4 MUX /CS outputs are set to HIGH (pins are active low)
    HAL_GPIO_WritePin(MUX_FA_CS_N_GPIO_Port,  MUX_FA_CS_N_Pin,  GPIO_PIN_SET); // Deselect Force-A MUX control
    HAL_GPIO_WritePin(MUX_FB_EN_N_GPIO_Port,  MUX_FB_EN_N_Pin,  GPIO_PIN_SET); // Deselect Force-B MUX control
    HAL_GPIO_WritePin(MUX_SA_EN_N_GPIO_Port,  MUX_SA_EN_N_Pin,  GPIO_PIN_SET); // Deselect Sense-A MUX control
    HAL_GPIO_WritePin(MUX_SB_EN_N_GPIO_Port,  MUX_SA_EN_N_Pin,  GPIO_PIN_SET); // Deselect Sense-B MUX control
    
    // De-activate the /WR for 4 MUXes. setting /WR to HIGH 
    // pin /WR is shared between 4 MUXes
    HAL_GPIO_WritePin(MUX_WR_N_GPIO_Port,  MUX_WR_N_Pin,  GPIO_PIN_SET); 

    // Set the shared MUX address (MUX 1 to 4) to 00000 , MUX  A4–A0  = 00000  (channel S1)
    // Shared address pins (A0 to A4) is shared between all 4 MUXes.
    HAL_GPIO_WritePin(MUX_A0_GPIO_Port , MUX_A0_Pin,  GPIO_PIN_SET);   // A0=0;
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port , MUX_A1_Pin,  GPIO_PIN_SET);   // A1=0;
    HAL_GPIO_WritePin(MUX_A2_GPIO_Port , MUX_A2_Pin,  GPIO_PIN_SET);   // A2=0;
    HAL_GPIO_WritePin(MUX_A3_GPIO_Port , MUX_A3_Pin,  GPIO_PIN_SET);   // A3=0;
    HAL_GPIO_WritePin(MUX_A4_GPIO_Port , MUX_A4_Pin,  GPIO_PIN_SET);   // A4=0;

    /*=================================================================================================*/
    // ************************************** Disable Precision ADC (ADS1220) **************************
    HAL_GPIO_WritePin(ADC_CS_N_GPIO_Port , ADC_CS_N_Pin,  GPIO_PIN_SET);   // Disable the ADS1220 by setting  ADC_CS_N to HIGH

    /*=================================================================================================*/
    // ************************************** Make display in-active ****************************

    HAL_GPIO_WritePin(LCD_CS_N_GPIO_Port, LCD_CS_N_Pin, GPIO_PIN_SET);        // LCD_CS_N to HIGH. 
    HAL_GPIO_WritePin(FLASH_CS_N_GPIO_Port, FLASH_CS_N_Pin, GPIO_PIN_SET);    // FLASH_CS_N to HIGH. 
    HAL_GPIO_WritePin(LCD_RESET_N_GPIO_Port, LCD_RESET_N_Pin, GPIO_PIN_SET);  // LCD_RESET_N to HIGH. 
    HAL_GPIO_WritePin(LCD_DCX_GPIO_Port, LCD_DCX_Pin, GPIO_PIN_SET);          // LCD_DCX to HIGH. 


   /*=================================================================================================*/
   // ************************************** Turn off 3 LED indicators and the buzzer *****************



    /* TODO: Set LED_FAIL LOW. */
    /* TODO: Set LED_PASS LOW. */
    /* TODO: Set LED_STATUS LOW. */
    /* TODO: Set BUZZER_EN LOW. */
}