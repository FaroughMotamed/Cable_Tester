#include "indicators.h"
#include "main.h"

// Sequence of events.
/*
1. A firmware event occurs.
2. indicator_start_pattern()
3. indicator_stop_pattern()
    - Stops and clears any existing pattern.
4. indicator_turn_off_all_leds()
    - Turns off the blue, green and red LEDs.
5. indicator_turn_on_selected_led()
6. indicator_set_pattern_output(true)
    - Starts the first ON phase.
    - Turns on the selected LED.
    - Turns on the buzzer if enabled.
7. indicator_process()
    - Called continuously from the main loop.
    - Checks total pattern time and current phase time.
8. indicator_set_pattern_output(false)
    - Called when the ON duration expires.
    - Turns off the LED and buzzer.
9. indicator_set_pattern_output(true)
    - Called when the OFF duration expires.
    - Starts the next ON phase.
10. indicator_process()
    - Continues alternating between ON and OFF phases.
11. indicator_stop_pattern()
    - Called when total pattern duration expires.
    - Turns off the LED and buzzer.
    - Marks the pattern inactive.
*/


#define INDICATOR_MAXIMUM_TOTAL_DURATION_MS  30000U

// Current pattern configuration.
static indicator_led_t active_led = INDICATOR_LED_NONE;

/*
 Indicates whether an LED/buzzer pattern is currently running.
 false:  
 No pattern is active. indicator_process() has nothing to do.

 true:
     A pattern has started and indicator_process() must continue checking its ON/OFF and total-duration timers.
 */
static bool pattern_active = false;


/*
 Records whether the pattern is currently in its ON phase or its OFF phase.
 
 false: The selected LED and buzzer are currently off.
 true: The selected LED and optional buzzer are currently on.
 
 indicator_process() uses this variable to determine whether
 it should compare elapsed time against pattern_on_duration_ms
 or pattern_off_duration_ms.
 */
static bool pattern_output_is_on = false;


/*
 Records whether the buzzer should follow the LED pattern.
 
 false:  Only the selected LED blinks. The buzzer remains off.
 true: The buzzer turns on and off at the same time as the LED.
 
 This value is set from the beep_enabled argument passed into
 indicator_start_pattern().
*/
static bool pattern_beep_enabled = false;


/*
 Stores the HAL tick value at which the complete indicator pattern started.
 
 It is used to calculate how long the complete pattern has
 been running:   HAL_GetTick() - pattern_start_time_ms
 When this elapsed time reaches pattern_total_duration_ms,
 the entire pattern is stopped.
 */
static uint32_t pattern_start_time_ms = 0U;


/*
 Stores the HAL tick value at which the current ON or OFF
 phase started.
 
 It is used to calculate how long the current phase has been
 running:   HAL_GetTick() - phase_start_time_ms
 This value is updated every time the pattern changes:  ON  -> OFF, OFF -> ON
 */
static uint32_t phase_start_time_ms = 0U;



// Stores how many milliseconds the selected LED and buzzer remain on during each pulse.
static uint32_t pattern_on_duration_ms = 0U;


/*
 Stores how many milliseconds the selected LED and buzzer remain off between pulses.
 A value of zero means that is no repeating OFF/ON cycle and the pattern acts as one                                                        
 continuous pulse until the total duration expires.
 */
static uint32_t pattern_off_duration_ms = 0U;


/*
 Stores the total duration of the complete indicator pattern.
 This includes every ON and OFF phase.
 Example:
     ON duration    = 200 ms
     OFF duration   = 300 ms
     Total duration = 3000 ms
 
 The LED and  buzzer alternate between ON and OFF
 for up to 3000 ms. After that, indicator_process() switches
 everything off and marks the pattern inactive.
 */
static uint32_t pattern_total_duration_ms = 0U;



// Turn off all three indicator LEDs.
static void indicator_turn_off_all_leds(void)
{
    HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_PASS_GPIO_Port,  LED_PASS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_FAIL_GPIO_Port, LED_FAIL_Pin, GPIO_PIN_RESET);
}



// Turn on the selected LED.
static bool indicator_turn_on_selected_led( indicator_led_t led)
{
    switch (led)
    {
        case INDICATOR_LED_NONE:
        {
            // A buzzer-only pattern is permitted.
            return true;
        }

        case INDICATOR_LED_BLUE:
        {
            HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_SET);
            return true;
        }

        case INDICATOR_LED_GREEN:
        {
            HAL_GPIO_WritePin(LED_PASS_GPIO_Port, LED_PASS_Pin, GPIO_PIN_SET);
            return true;
        }

        case INDICATOR_LED_RED:
        {
            HAL_GPIO_WritePin(LED_FAIL_GPIO_Port, LED_FAIL_Pin, GPIO_PIN_SET);
            return true;
        }

        default:
        {
            return false;
        }
    }
}



// Apply the requested ON or OFF state to the selected LED and the buzzer.

static void indicator_set_pattern_output(bool output_on)
{
    if (output_on)
    {
        
        // Make sure only the selected LED is active.
        indicator_turn_off_all_leds();

        indicator_turn_on_selected_led(active_led);

        if (pattern_beep_enabled)
        {
            HAL_GPIO_WritePin(BUZZER_EN_GPIO_Port, BUZZER_EN_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(BUZZER_EN_GPIO_Port, BUZZER_EN_Pin, GPIO_PIN_RESET);
        }
    }
    else
    {
        
        // Turn off all LEDs and the buzzer during the OFF part of the pattern.
        indicator_turn_off_all_leds();
        HAL_GPIO_WritePin(BUZZER_EN_GPIO_Port, BUZZER_EN_Pin, GPIO_PIN_RESET);
    }

    pattern_output_is_on = output_on;
}



// Start a repeating LED and  buzzer pattern.
bool indicator_start_pattern(indicator_led_t led,
                             bool beep_enabled,
                             uint32_t on_duration_ms,
                             uint32_t off_duration_ms,
                             uint32_t total_duration_ms)
{
    uint32_t current_time_ms;

    
    // Each ON portion must have a nonzero duration.
    if (on_duration_ms == 0U || total_duration_ms == 0U )
    {
        return false;
    }

    // Reject an unreasonably long pattern.
    if (total_duration_ms > INDICATOR_MAXIMUM_TOTAL_DURATION_MS)
    {
        return false;
    }




    // Turn off the previous pattern before replacing it.
    indicator_stop_pattern();

    current_time_ms = HAL_GetTick();

    // Save the new pattern configuration.
    active_led = led;
    pattern_beep_enabled = beep_enabled;

    pattern_on_duration_ms = on_duration_ms;
    pattern_off_duration_ms = off_duration_ms;
    pattern_total_duration_ms = total_duration_ms;

    pattern_start_time_ms = current_time_ms;
    phase_start_time_ms = current_time_ms;

    pattern_active = true;


    // Every new pattern begins with its ON phase.
    indicator_set_pattern_output(true);

    return true;
}



// Process the current LED/buzzer pattern without waiting.
// Call this function continuously from the main loop.
void indicator_process(void)
{
    uint32_t current_time_ms;
    uint32_t total_elapsed_ms;
    uint32_t phase_elapsed_ms;

    if (!pattern_active)
    {
        return;
    }

    current_time_ms = HAL_GetTick();

 
    // Determine how long the complete pattern has run.
    total_elapsed_ms = current_time_ms - pattern_start_time_ms;


    // Stop when the requested total duration expires.
    if (total_elapsed_ms >= pattern_total_duration_ms)
    {
        indicator_stop_pattern();
        return;
    }

    
    // Determine how long the current ON or OFF phase has been active.
    phase_elapsed_ms = current_time_ms - phase_start_time_ms;

    if (pattern_output_is_on)
    {
        // The ON phase has finished.
        if (phase_elapsed_ms >= pattern_on_duration_ms)
        {
            indicator_set_pattern_output(false);
            phase_start_time_ms = current_time_ms;
        }
    }
    else
    {
        
        // If the OFF duration is zero, the pattern is a single continuous pulse rather than a repeating blink.
        if (pattern_off_duration_ms == 0U)
        {
            return;
        }

 
        // The OFF phase has finished. Begin another ON phase.
        if (phase_elapsed_ms >= pattern_off_duration_ms)
        {
            indicator_set_pattern_output(true);
            phase_start_time_ms = current_time_ms;
        }
    }
}



// Immediately stop the active pattern.
void indicator_stop_pattern(void)
{
    indicator_turn_off_all_leds();

    HAL_GPIO_WritePin(BUZZER_EN_GPIO_Port, BUZZER_EN_Pin, GPIO_PIN_RESET);

    pattern_active = false;
    pattern_output_is_on = false;
    pattern_beep_enabled = false;

    pattern_on_duration_ms = 0U;
    pattern_off_duration_ms = 0U;
    pattern_total_duration_ms = 0U;

    active_led = INDICATOR_LED_NONE;
}



// Report whether a pattern is currently running.
bool indicator_pattern_is_active(void)
{
    return pattern_active;
}