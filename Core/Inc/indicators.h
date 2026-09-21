#ifndef INDICATORS_H
#define INDICATORS_H

#include <stdbool.h>
#include <stdint.h>


// Only one LED can be selected for each indicator pattern.
typedef enum
{
    INDICATOR_LED_NONE = 0,
    INDICATOR_LED_BLUE,
    INDICATOR_LED_GREEN,
    INDICATOR_LED_RED

} indicator_led_t;


/*
Independent timing configuration for one output.

This structure is used separately for:
1. The selected LED
2. The buzzer

enabled:
    false = output is disabled.
    true  = output follows the timing settings.

start_delay_ms:
    Delay before this output starts.

on_duration_ms:
    Length of each ON period.

off_duration_ms:
    Length of each OFF period.

    Set this to zero to keep the output continuously
    on until total_duration_ms expires.

total_duration_ms:
    Total duration after the start delay.
*/
typedef struct
{
    bool enabled;

    uint32_t start_delay_ms;
    uint32_t on_duration_ms;
    uint32_t off_duration_ms;
    uint32_t total_duration_ms;

} indicator_output_pattern_t;


/*
Start one independently timed LED and buzzer pattern.

led:
    Selects one LED.

led_pattern:
    Independent timing configuration for the selected LED.

beep_pattern:
    Independent timing configuration for the buzzer.

Starting a new pattern stops and replaces any existing pattern.

Returns:
    true  = pattern successfully started.
    false = invalid arguments or timing configuration.
*/
bool indicator_start_pattern(
    indicator_led_t led,
    const indicator_output_pattern_t *led_pattern,
    const indicator_output_pattern_t *beep_pattern);


// Call continuously from the main loop.
void indicator_process(void);


// Immediately stop the active pattern.
void indicator_stop_pattern(void);


// Returns true while an indicator pattern is active.
bool indicator_pattern_is_active(void);


#endif // INDICATORS_H