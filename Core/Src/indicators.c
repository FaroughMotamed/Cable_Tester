#include "indicators.h"
#include "main.h"
#include <stddef.h>

// This code independently controls one LED and the buzzer using a single call to indicator_start_pattern()

/* ===================  This is the sequence of functions calls that happen to control LED and buzzer  ===================

Firmware event like test success generates the following sequence:

- Create LED and buzzer timing configurations
- indicator_start_pattern()
- indicator_stop_pattern() clears old pattern
- Save new settings and starting time
- indicator_process()
- indicator_calculate_output_state() for LED
- indicator_calculate_output_state() for buzzer
- indicator_set_led_output()
- indicator_set_beep_output()
- Repeat indicator_process()
- Both outputs finish
- indicator_stop_pattern()

*/


/* ===================  Detailed desription of the sequence of events  ===================

Sequence of events:

1. A firmware event occurs.
    - Encoder change, test pass, test failure or hardware fault.

2. The firmware creates two timing configurations.
    - One for the selected LED.
    - One for the buzzer.
    - Each has its own start delay, ON time, OFF time and total time.

3. indicator_start_pattern() is called.
    - Validates the LED and both timing configurations.
    - Calls indicator_stop_pattern() to cancel any previous pattern.
    - Copies the new LED and buzzer settings.
    - Saves the common starting time.
    - Sets pattern_active to true.
    - Initially turns off the LED and buzzer.
    - Returns true if the pattern starts successfully.

4. The main loop continuously calls indicator_process().
    - Returns immediately if no pattern is active.
    - Calculates the time elapsed since the pattern started.

5. indicator_calculate_output_state() calculates the timings for the LED.
    - Waits for the LED start delay.
    - Calculates whether the LED should be ON or OFF.
    - Reports when the LED pattern has finished.

6. indicator_calculate_output_state() calculates the timings for the buzzer separately.
    - Waits for the buzzer start delay.
    - Calculates whether the buzzer should be ON or OFF.
    - Reports when the buzzer pattern has finished.

7. indicator_set_led_output() applies the calculated LED state.
    - Turns the selected LED ON or OFF.

8. indicator_set_beep_output() applies the calculated buzzer state.
    - Turns the buzzer ON or OFF.

9. indicator_process() checks whether both outputs have finished.
    - If one output is still active, processing continues.
    - If both outputs are finished, it calls indicator_stop_pattern().

10. indicator_stop_pattern() ends the sequence.
    - Turns off all LEDs.
    - Turns off the buzzer.
    - Clears the saved timing configurations.
    - Sets pattern_active to false.


*/



// Maximum permitted value for each timing parameter.
#define INDICATOR_MAXIMUM_TIME_MS  60000U


/*
LED selected for the currently active pattern.

Only one LED can be controlled by this module at a time.
*/
static indicator_led_t active_led = INDICATOR_LED_NONE;


/*
Independent timing configurations for the selected
LED and the buzzer.
*/
static indicator_output_pattern_t active_led_pattern;
static indicator_output_pattern_t active_beep_pattern;


// Indicates whether the indicator system is running a pattern.
static bool pattern_active = false;


/*
Common starting time for the complete pattern.

The LED and buzzer use this same reference time, but
each applies its own start_delay_ms.
*/
static uint32_t pattern_start_time_ms = 0U;


/*
Turn off all three indicator LEDs.

This assumes active-high transistor drivers:
GPIO HIGH = LED on
GPIO LOW  = LED off
*/
static void indicator_turn_off_all_leds(void)
{
    HAL_GPIO_WritePin( LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin( LED_PASS_GPIO_Port, LED_PASS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(  LED_FAIL_GPIO_Port, LED_FAIL_Pin, GPIO_PIN_RESET);
}


/*
Apply an ON or OFF state to the selected LED.

All LEDs are turned off first. The selected LED is then
turned on if output_on is true.
*/
static void indicator_set_led_output(bool output_on)
{
    // Make sure all unselected LEDs remain off.
    indicator_turn_off_all_leds();

    // Leave every LED off when output_on is false.
    if (!output_on)
    {
        return;
    }

    switch (active_led)
    {
        case INDICATOR_LED_BLUE:
        {
            HAL_GPIO_WritePin( LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_SET);

            break;
        }

        case INDICATOR_LED_GREEN:
        {
            HAL_GPIO_WritePin( LED_PASS_GPIO_Port, LED_PASS_Pin, GPIO_PIN_SET);

            break;
        }

        case INDICATOR_LED_RED:
        {
            HAL_GPIO_WritePin( LED_FAIL_GPIO_Port, LED_FAIL_Pin, GPIO_PIN_SET);

            break;
        }

        case INDICATOR_LED_NONE:
        {
            // No LED is selected.
            break;
        }

        default:
        {
            // Invalid values are handled during pattern validation.
            break;
        }
    }
}


/*
Apply an ON or OFF state to the buzzer.

This assumes an active buzzer controlled by an
active-high transistor driver.
*/
static void indicator_set_beep_output(bool output_on)
{
    if (output_on)
    {
        HAL_GPIO_WritePin( BUZZER_EN_GPIO_Port, BUZZER_EN_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin( BUZZER_EN_GPIO_Port,  BUZZER_EN_Pin, GPIO_PIN_RESET);
    }
}


// Check whether the requested LED selection is valid.
static bool indicator_led_is_valid(indicator_led_t led)
{
    switch (led)
    {
        case INDICATOR_LED_NONE:
        {
            return true;
        }

        case INDICATOR_LED_BLUE:
        {
            return true;
        }

        case INDICATOR_LED_GREEN:
        {
            return true;
        }

        case INDICATOR_LED_RED:
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}


/*
Validate one LED or buzzer timing configuration.

Disabled outputs do not require valid timing values.
Enabled outputs require nonzero ON and total durations.
*/
static bool indicator_output_pattern_is_valid(
    const indicator_output_pattern_t *pattern)
{
    // Reject an invalid configuration pointer.
    if (pattern == NULL)
    {
        return false;
    }

    // A disabled output does not require timing values.
    if (!pattern->enabled)
    {
        return true;
    }

    // An enabled output requires a nonzero ON duration.
    if (pattern->on_duration_ms == 0U)
    {
        return false;
    }

    // An enabled output requires a nonzero total duration.
    if (pattern->total_duration_ms == 0U)
    {
        return false;
    }

    // Validate the start delay.
    if (pattern->start_delay_ms >
        INDICATOR_MAXIMUM_TIME_MS)
    {
        return false;
    }

    // Validate the ON duration.
    if (pattern->on_duration_ms >
        INDICATOR_MAXIMUM_TIME_MS)
    {
        return false;
    }

    // Validate the OFF duration.
    if (pattern->off_duration_ms >
        INDICATOR_MAXIMUM_TIME_MS)
    {
        return false;
    }

    // Validate the total duration.
    if (pattern->total_duration_ms >
        INDICATOR_MAXIMUM_TIME_MS)
    {
        return false;
    }

    return true;
}


/*
Calculate whether one independently timed output should currently be ON or OFF.

elapsed_pattern_ms:
    Time elapsed since indicator_start_pattern was called.

pattern:
    Timing configuration for this output.

output_finished:
    Receives true when this output has completed.

Returns:
    true  = output should currently be ON.
    false = output should currently be OFF.
*/
static bool indicator_calculate_output_state(
    uint32_t elapsed_pattern_ms,
    const indicator_output_pattern_t *pattern,
    bool *output_finished)
{
    uint32_t output_elapsed_ms;
    uint32_t cycle_duration_ms;
    uint32_t cycle_position_ms;

    // A disabled output remains off and is already finished.
    if (!pattern->enabled)
    {
        *output_finished = true;
        return false;
    }

    // Keep the output off while waiting for its start delay.
    if (elapsed_pattern_ms < pattern->start_delay_ms)
    {
        *output_finished = false;
        return false;
    }

    // Calculate elapsed time since this output started.
    output_elapsed_ms =
        elapsed_pattern_ms - pattern->start_delay_ms;

    // Stop the output after its total duration expires.
    if (output_elapsed_ms >= pattern->total_duration_ms)
    {
        *output_finished = true;
        return false;
    }

    // The output is active and has not finished.
    *output_finished = false;

    /*
    An OFF duration of zero means continuous operation.
    The output remains on until total_duration_ms expires.
    */
    if (pattern->off_duration_ms == 0U)
    {
        return true;
    }

    // Calculate the length of one complete ON and OFF cycle.
    cycle_duration_ms =
        pattern->on_duration_ms +
        pattern->off_duration_ms;

    // Calculate the current position inside the cycle.
    cycle_position_ms =
        output_elapsed_ms % cycle_duration_ms;

    // Every cycle begins with its ON phase.
    if (cycle_position_ms < pattern->on_duration_ms)
    {
        return true;
    }

    // The output is currently in its OFF phase.
    return false;
}


/*
Start independently timed LED and buzzer patterns.

The timing configurations are copied into static storage.
The caller's structures do not need to remain available
after this function returns.
*/
bool indicator_start_pattern(
    indicator_led_t led,
    const indicator_output_pattern_t *led_pattern,
    const indicator_output_pattern_t *beep_pattern)
{
    // Both configuration pointers must be valid.
    if ((led_pattern == NULL) ||
        (beep_pattern == NULL))
    {
        return false;
    }

    // Validate the selected LED.
    if (!indicator_led_is_valid(led))
    {
        return false;
    }

    // An enabled LED pattern requires an actual selected LED.
    if (led_pattern->enabled &&
        (led == INDICATOR_LED_NONE))
    {
        return false;
    }

    // Validate the LED timing configuration.
    if (!indicator_output_pattern_is_valid(led_pattern))
    {
        return false;
    }

    // Validate the buzzer timing configuration.
    if (!indicator_output_pattern_is_valid(beep_pattern))
    {
        return false;
    }

    // At least one output must be enabled.
    if (!led_pattern->enabled &&
        !beep_pattern->enabled)
    {
        return false;
    }

    // Stop and replace any currently active pattern.
    indicator_stop_pattern();

    // Save the selected LED.
    active_led = led;

    // Copy the LED timing configuration.
    active_led_pattern = *led_pattern;

    // Copy the buzzer timing configuration.
    active_beep_pattern = *beep_pattern;

    // Save the common pattern start time.
    pattern_start_time_ms = HAL_GetTick();

    // Mark the pattern as active.
    pattern_active = true;

    /*
    Start with both outputs off.

    indicator_process applies the correct output states
    on the next main-loop iteration.
    */
    indicator_set_led_output(false);
    indicator_set_beep_output(false);

    return true;
}


/*
Update the independently timed LED and buzzer outputs.

This function does not wait and does not use HAL_Delay.
Call it continuously from the main loop.
*/
void indicator_process(void)
{
    uint32_t elapsed_pattern_ms;

    bool led_output_on;
    bool beep_output_on;

    bool led_finished;
    bool beep_finished;

    // Nothing needs to be processed if no pattern is active.
    if (!pattern_active)
    {
        return;
    }

    /*
    Calculate elapsed time since the complete pattern began.

    Unsigned subtraction remains correct when HAL_GetTick
    eventually wraps around.
    */
    elapsed_pattern_ms =
        HAL_GetTick() - pattern_start_time_ms;

    // Calculate the current LED state.
    led_output_on =
        indicator_calculate_output_state(
            elapsed_pattern_ms,
            &active_led_pattern,
            &led_finished);

    // Calculate the current buzzer state.
    beep_output_on =
        indicator_calculate_output_state(
            elapsed_pattern_ms,
            &active_beep_pattern,
            &beep_finished);

    // Apply the calculated LED state.
    indicator_set_led_output(led_output_on);

    // Apply the calculated buzzer state.
    indicator_set_beep_output(beep_output_on);

    /*
    The complete pattern finishes only after both
    independent outputs have finished.
    */
    if (led_finished && beep_finished)
    {
        indicator_stop_pattern();
    }
}


// Immediately stop the active pattern.
void indicator_stop_pattern(void)
{
    // Place all LEDs in their safe OFF state.
    indicator_turn_off_all_leds();

    // Place the buzzer in its safe OFF state.
    indicator_set_beep_output(false);

    // Clear the overall pattern state.
    pattern_active = false;
    pattern_start_time_ms = 0U;

    // Clear the selected LED.
    active_led = INDICATOR_LED_NONE;

    // Clear the saved LED configuration.
    active_led_pattern.enabled = false;
    active_led_pattern.start_delay_ms = 0U;
    active_led_pattern.on_duration_ms = 0U;
    active_led_pattern.off_duration_ms = 0U;
    active_led_pattern.total_duration_ms = 0U;

    // Clear the saved buzzer configuration.
    active_beep_pattern.enabled = false;
    active_beep_pattern.start_delay_ms = 0U;
    active_beep_pattern.on_duration_ms = 0U;
    active_beep_pattern.off_duration_ms = 0U;
    active_beep_pattern.total_duration_ms = 0U;
}


// Report whether an indicator pattern is currently active.
bool indicator_pattern_is_active(void)
{
    return pattern_active;
}