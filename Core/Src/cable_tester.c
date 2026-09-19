#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "ads1220.h"
#include "cable_tester.h"


  // Different paramters used for disabling the muxes are defined in the main.h

  /*=====================================================================================================*/
  // *****************************************  Disable 4 MUXes ******************************************

  // Set enable pins to high
  // 4 MUX /EN pins are set to HIGH (pins are active low)
  void disable_all_muxes(void){
  HAL_GPIO_WritePin(MUX_FA_EN_N_GPIO_Port, MUX_FA_EN_N_Pin, GPIO_PIN_SET); // Disable Force-A MUX
  HAL_GPIO_WritePin(MUX_FB_EN_N_GPIO_Port, MUX_FB_EN_N_Pin, GPIO_PIN_SET); // Disable Force-B MUX
  HAL_GPIO_WritePin(MUX_SA_EN_N_GPIO_Port, MUX_SA_EN_N_Pin, GPIO_PIN_SET); // Disable Sense-A MUX
  HAL_GPIO_WritePin(MUX_SB_EN_N_GPIO_Port, MUX_SB_EN_N_Pin, GPIO_PIN_SET); // Disable Sense-B MUX
  };

  // Deselect all four individual MUX control interfaces.
  // 4 MUX /CS outputs are set to HIGH (pins are active low)
  void deselect_all_muxes(void){
  HAL_GPIO_WritePin(MUX_FA_CS_N_GPIO_Port, MUX_FA_CS_N_Pin, GPIO_PIN_SET); // Deselect Force-A MUX control
  HAL_GPIO_WritePin(MUX_FB_CS_N_GPIO_Port, MUX_FB_CS_N_Pin, GPIO_PIN_SET); // Deselect Force-B MUX control
  HAL_GPIO_WritePin(MUX_SA_CS_N_GPIO_Port, MUX_SA_CS_N_Pin, GPIO_PIN_SET); // Deselect Sense-A MUX control
  HAL_GPIO_WritePin(MUX_SB_CS_N_GPIO_Port, MUX_SB_CS_N_Pin, GPIO_PIN_SET); // Deselect Sense-B MUX control
  };


  // De-activate the /WR for 4 MUXes. setting /WR to HIGH
  // pin /WR is shared between 4 MUXes
  void deactivate_write_all_muxes(void){
  HAL_GPIO_WritePin(MUX_WR_N_GPIO_Port, MUX_WR_N_Pin, GPIO_PIN_SET);
  };

  // Set the shared MUX address (MUX 1 to 4) to 00000 , MUX  A4–A0  = 00000  (channel S1)
  // Shared address pins (A0 to A4) is shared between all 4 MUXes.
  void set_shared_mux_address_all_zero (void) {
  HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, GPIO_PIN_RESET); // A0=0;
  HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, GPIO_PIN_RESET); // A1=0;
  HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, GPIO_PIN_RESET); // A2=0;
  HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, GPIO_PIN_RESET); // A3=0;
  HAL_GPIO_WritePin(MUX_A4_GPIO_Port, MUX_A4_Pin, GPIO_PIN_RESET); // A4=0;
  };

  /*=================================================================================================*/
  // ************************************** Deselect  ADS1220 (ADC) SPI interface (ADS1220) **************************
  void deselect_ads1220_spi_interface(void){
  HAL_GPIO_WritePin(ADC_CS_N_GPIO_Port, ADC_CS_N_Pin, GPIO_PIN_SET); // Deselect  ADS1220 SPI by setting  ADC_CS_N to HIGH
  };

  /*=================================================================================================*/
  // ************************************** Make display in-active ****************************
  void deactivate_display(void){
  HAL_GPIO_WritePin(LCD_CS_N_GPIO_Port, LCD_CS_N_Pin, GPIO_PIN_SET);       // LCD_CS_N to HIGH.
  HAL_GPIO_WritePin(FLASH_CS_N_GPIO_Port, FLASH_CS_N_Pin, GPIO_PIN_SET);   // FLASH_CS_N to HIGH.
  HAL_GPIO_WritePin(LCD_RESET_N_GPIO_Port, LCD_RESET_N_Pin, GPIO_PIN_SET); // LCD_RESET_N to HIGH.
  HAL_GPIO_WritePin(LCD_DCX_GPIO_Port, LCD_DCX_Pin, GPIO_PIN_RESET);       // LCD_DCX to LOW (command mode not data mode).
  };

  /*=================================================================================================*/
  // ************************************** Turn off 3 LED indicators and the buzzer *****************
  void turn_off_buzzer_indicators(void){
  HAL_GPIO_WritePin(LED_FAIL_GPIO_Port, LED_FAIL_Pin, GPIO_PIN_RESET);     // Set RED    LED_FAIL    to LOW
  HAL_GPIO_WritePin(LED_PASS_GPIO_Port, LED_PASS_Pin, GPIO_PIN_RESET);     // Set GREEN  LED_PASS    to LOW
  HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_RESET); // Set BLUE   LED_STATUS  to LOW
  HAL_GPIO_WritePin(BUZZER_EN_GPIO_Port, BUZZER_EN_Pin, GPIO_PIN_RESET);   // Set BUZZER             to LOW
  };

  // LATER:  set state flgas here.


void enter_safe_state(void)
{
  disable_all_muxes();
  deselect_all_muxes();
  deactivate_write_all_muxes();
  set_shared_mux_address_all_zero();
  deselect_ads1220_spi_interface();
  deactivate_display();
  turn_off_buzzer_indicators();
};


// Enum for different LED, buzzer indicator modes
typedef enum
{
  INDICATOR_OFF = 0,
  INDICATOR_INIT,
  INDICATOR_READY,
  INDICATOR_TESTING,
  INDICATOR_PASS,
  INDICATOR_CALIB,
  INDICATOR_FAIL,
  INDICATOR_ERROR

} indicator_mode;


// set indcator states
void set_indicator_mode(indicator_mode mode)
{
  switch (mode)
  {
    case INDICATOR_OFF:
            printf("Indicator OFF");
            HAL_GPIO_WritePin(LED_FAIL_GPIO_Port,   LED_FAIL_Pin,   GPIO_PIN_RESET);     // Set RED    LED_FAIL    to LOW
            HAL_GPIO_WritePin(LED_PASS_GPIO_Port,   LED_PASS_Pin,   GPIO_PIN_RESET);     // Set GREEN  LED_PASS    to LOW
            HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_RESET);     // Set BLUE   LED_STATUS  to LOW
            HAL_GPIO_WritePin(BUZZER_EN_GPIO_Port,  BUZZER_EN_Pin,  GPIO_PIN_RESET);     // Set BUZZER  to LOW
      break;

    case INDICATOR_INIT:
      printf("Indicator INIT");
            //HAL_GPIO_WritePin(LED_FAIL_GPIO_Port,   LED_FAIL_Pin, GPIO_PIN_RESET);     // Set RED    LED_FAIL    to LOW
            //HAL_GPIO_WritePin(LED_PASS_GPIO_Port,   LED_PASS_Pin, GPIO_PIN_RESET);     // Set GREEN  LED_PASS    to LOW
              HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_SET);     // Set BLUE   LED_STATUS  to HIGH
            //HAL_GPIO_WritePin(BUZZER_EN_GPIO_Port,  BUZZER_EN_Pin, GPIO_PIN_RESET);    // Set BUZZER  to LOW
      break;

    case INDICATOR_READY:
      printf("Indicator READY");
      break;

    case INDICATOR_TESTING:
      printf("Indicator TESTING");
      break;

    case INDICATOR_PASS:
      printf("Indicator PASS");
      break;

    case INDICATOR_CALIB:
      printf("Indicator CALIB");
      break;

    case INDICATOR_FAIL:
      printf("Indicator FAIL");
      break;

    case INDICATOR_ERROR:
      printf("Indicator ERROR");
      break;
  }
};






// Define the cable_tester_init
void cable_tester_init(void)
{

  enter_safe_state();
  set_indicator_mode(INDICATOR_INIT);


/* Nest steps for the init process of the device:
Enter safe state
set indicator mode
Initialize ADS1220 (precision ADC, Analoge Digital Converter )
Load calibration from Flash
Initialize  encoder
Read the selector
Initialize  display
Verify current source
Enter READY state
*/

}


/*
 Measure the voltage across the cable.
 AIN0 is the positive input.
 AIN1 is the negative input.
 cable_voltage receives AIN0 - AIN1 in volts.
 */
bool measure_cable_voltage(float *cable_voltage){

  bool output= ads1220_measure_input(ADS1220_CABLE_SELECT, cable_voltage);
  return output;
}



/*
 Measure the voltage across the 200.0 ohm resistor;
 AIN2 is the positive input.
 AIN3 is the negative input.
 sense_voltage receives AIN2 - AIN3 in volts.
*/
bool measure_sense_voltage(float *sense_voltage){

  bool output= ads1220_measure_input(ADS1220_RESISTOR_SELECT, sense_voltage);
  return output;
}


// Determine cable presence from the sense-resistor voltage.
// open cable generates no current and no voltage difference across resistor.
// closed cable generates 2.048V/200 ohm=10.24 mA current and 2.028 Voltage differnece.
// True meanscables present, False means there is no cable. 

#define CABLE_PRESENT_SENSE_THRESHOLD_V     1.500f
#define SENSE_VOLTAGE_MAXIMUM_V             2.500f

// Determine cable presence from the sense-resistor voltage.
// open cable generates no current and no voltage difference across resistor.
// closed cable generates 2.048V/200 ohm=10.24 mA current and 2.048 Voltage differnece.

// ads1220 fault, cable open: false
// cable close: true
bool check_cable_presence()
{
    float sense_voltage;

    // Measure AIN2 - AIN3 across the 200-ohm resistor.
    if (!measure_sense_voltage(&sense_voltage))
    {
        return false;
    }

    // A negative voltage or an unexpectedly high voltage
    // indicates a wiring, polarity or current-source problem.
    if ((sense_voltage < 0.0f) || (sense_voltage > SENSE_VOLTAGE_MAXIMUM_V))
    {
        return false;
    }

    /*
     A closed cable path should allow the constant current
     to flow, producing approximately 2.048 V across the
     200-ohm resistor.
     */
    if (sense_voltage >= CABLE_PRESENT_SENSE_THRESHOLD_V)
    {
        //*status = CABLE_PRESENT;
        return true;
    }
    else
    {
        //*status = CABLE_NOT_PRESENT;
        return false;
    }
}









