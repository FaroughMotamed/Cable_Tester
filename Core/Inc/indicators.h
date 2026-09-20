#ifndef INDICATORS_H
#define INDICATORS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    INDICATOR_LED_NONE = 0,
    INDICATOR_LED_BLUE,
    INDICATOR_LED_GREEN,
    INDICATOR_LED_RED

} indicator_led_t;


/*
 Start a nonblocking LED and buzzer pattern.
 
 led:
     LED that will blink.
 
 beep_enabled:
     true  = buzzer follows the LED on/off pattern.
     false = only the selected LED blinks.
 
 on_duration_ms:
     Time that the LED and  buzzer remain on
     during each pulse.
 
 off_duration_ms:
     Time that the LED and  buzzer remain off between pulses.
 
 total_duration_ms:
     Total time that the complete pattern continues.
 
 Example:
     on_duration_ms    = 200
     off_duration_ms   = 200
     total_duration_ms = 3000
 
 Result:
     ON  200 ms
     OFF 200 ms
     ...
     Stop after 3 seconds
 Starting a new pattern replaces any currently active pattern.
 */
bool indicator_start_pattern(indicator_led_t led,   bool beep_enabled,
                                                    uint32_t on_duration_ms,
                                                    uint32_t off_duration_ms,
                                                    uint32_t total_duration_ms);



// Call continuously from the main loop.
void indicator_process(void);

// Immediately stop the current pattern.
void indicator_stop_pattern(void);

// Returns true while an indicator pattern is active.
bool indicator_pattern_is_active(void);

#endif /* INDICATORS_H */