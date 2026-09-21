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




// Determine cable presence from the sense-resistor voltage.
// open cable generates no current and no voltage difference across resistor.
// closed cable generates 2.048V/200 ohm=10.24 mA current and 2.028 Voltage differnece.
// True meanscables present, False means there is no cable. 

#define CABLE_PRESENT_SENSE_THRESHOLD_V     1.500f
#define SENSE_VOLTAGE_MAXIMUM_V             2.500f

typedef enum{
CABLE_PRESENT =0, 
CABLE_NOT_PRESENT = 1,
ADS1220_FAULT=2,
FALSE_MEASUREMENT=3
} cable_presence_status_t;

#define CABLE_PRESENCE_CHECK_INTERVAL_MS  100.0f

// Determine cable presence from the sense-resistor voltage.
// open cable generates no current and no voltage difference across resistor.
// closed cable generates 2.048V/200 ohm=10.24 mA current and 2.048 Voltage differnece.

// ads1220 fault, cable open: false
// cable close: true
bool check_cable_presence(cable_presence_status_t *status)
{
    float sense_voltage;

    // Measure AIN2 - AIN3 across the 200-ohm resistor.
    if (!measure_sense_voltage(&sense_voltage))
    {
        status = ADS1220_FAULT;
        return false;
    }

    // A negative voltage or an unexpectedly high voltage
    // indicates a wiring, polarity or current-source problem.
    if ((sense_voltage < 0.0f) || (sense_voltage > SENSE_VOLTAGE_MAXIMUM_V))
    {
        status = FALSE_MEASUREMENT;
        return false;
    }

    /*
     A closed cable path should allow the constant current
     to flow, producing approximately 2.048 V across the
     200-ohm resistor.
    */
    if (sense_voltage >= CABLE_PRESENT_SENSE_THRESHOLD_V)
    {
        status = CABLE_PRESENT;
        return true;
    }
    else
    {
        status = CABLE_NOT_PRESENT;
        return false;
    }
}



typedef enum
{
    TESTER_STATE_INIT = 0,
    TESTER_STATE_READY,
    TESTER_STATE_TESTING,
    TESTER_STATE_RESULTS,
    TESTER_STATE_CALIBRATION,
    TESTER_STATE_ERROR

} cable_tester_state_t;



/*
 Periodically check whether a cable is present.
 This check only happens in TESTER_STATE_READY.

 This function should be called repeatedly from
 cable_tester_process() or from the main loop.
 
 A new cable-presence measurement is performed:
     1. Only when presence_check_allowed is true.
     2. Immediately on the first allowed call.
     3. Once every 100 ms after the first check.
 
 The function does not block the main loop by using HAL_Delay(100). 
 If 100 ms has not elapsed, it immediately returns the previously saved
 cable-presence result.
 
 Parameters:
     presence_check_allowed:
         true  = background presence measurement is allowed.
         false = do not access the ADS1220 or measurement path.

     status:
         Receives the most recent detailed status:
             CABLE_PRESENT
             CABLE_NOT_PRESENT
             ADS1220_FAULT
             FALSE_MEASUREMENT
 
     new_result_f:
         Receives true when a new ADS1220 measurement was performed during this function call.
         Receives false when the function only returned the previously stored result.
 
 Return value:
     true:
         The most recent result indicates that a cable is present.
 
     false:
         The cable is absent, an ADS1220 failure occurred,
         the measurement was invalid, or an argument was invalid.

 Important:
     The Boolean return value only answers whether a cable is
     present. Check *status to distinguish an open cable from
     a measurement or ADS1220 fault.
 */

bool cable_presence_process( bool presence_check_allowed, cable_presence_status_t *status, bool *new_result_f)
{
  
    // Time at which the previous presence measurement started.
    //  Static variable retain its value between calls.
    static uint32_t previous_check_time_ms = 0U;

    // Most recently determined cable status.
    static cable_presence_status_t previous_status = CABLE_NOT_PRESENT;

    //Boolean form of the most recent cable-presence result.
    static bool previous_cable_presence = false;

    // This causes the first allowed check to happen immediately,
    // without waiting for the initial 100-ms interval.
    static bool first_check_required = true;

    uint32_t current_time_ms;

    //The function cannot return detailed information if either output pointer is invalid.
    if ((status == NULL) || (new_result_f == NULL))
    {
        return false;
    }

    // Unless a measurement is performed below, this call has not produced a new result.
    *new_result_f = false;

    // Initially return the status saved from the previous measurement.
    *status = previous_status;

    /*
     Do not access the ADS1220 unless presence checking is
     permitted by the cable tester state machine.
     
     This prevents the background presence check from
     interfering with initialization, testing, calibration,
     results processing or error handling.
    */
    if (!presence_check_allowed)
    {
        return previous_cable_presence;
    }

    // Obtain the number of milliseconds elapsed since HAL_Init().
    current_time_ms = HAL_GetTick();

    /*
     After the first measurement, wait until at least 100 ms
     has elapsed before starting another presence measurement.
     
     This is not a blocking wait. If 100 ms has not elapsed,
     the function immediately returns the cached result.
     
     Unsigned subtraction remains valid if HAL_GetTick() wraps around.
     */
    if ( !first_check_required &&  ( (current_time_ms - previous_check_time_ms) <  CABLE_PRESENCE_CHECK_INTERVAL_MS) )
    {
        return previous_cable_presence;
    }

    /*
     A new measurement is about to begin.
     Save its starting time so another background check will not begin for at least 100 ms.
    */
    previous_check_time_ms = current_time_ms;
    first_check_required = false;

    /*
     Perform the actual ADS1220 cable-presence measurement.
     This updates both:
         previous_cable_presence
         previous_status
    */
    previous_cable_presence = check_cable_presence(&previous_status);

    // Copy the newly calculated status to the caller.
    *status = previous_status;

    // Tell the caller that a new measurement was completed during this call. (even if the measurement failed)
    *new_result_f = true;

     //Return true only when the most recent measurement indicates that a cable is present.
    return previous_cable_presence;
}








