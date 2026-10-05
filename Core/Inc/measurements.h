
#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <stdbool.h>


/*
 Measure the voltage across the cable.
 AIN0 is the positive input-  AIN1 is the negative input.
 cable_voltage receives AIN0 - AIN1 in volts.
 */
bool measure_cable_voltage(float *cable_voltage);

/*
 Measure the voltage across the 200.0 ohm resistor;
 AIN2 is the positive input- AIN3 is the negative input.
 sense_voltage receives AIN2 - AIN3 in volts.
*/
bool measure_sense_voltage(float *sense_voltage);


/*
Measure the cable voltage and sense-resistor voltage.

cable_voltage: Receives AIN0 - AIN1 in volts.
sense_voltage: Receives AIN2 - AIN3 in volts.

Returns:
    true  = both voltage measurements succeeded.
    false = an output pointer was invalid or an ADS1220 measurement failed.
*/
bool measure_voltages(float *cable_voltage, float *sense_voltage);

/*
Calculate the current flowing through the 200-ohm precision sense resistor.

sense_voltage: Measured voltage across the sense resistor in volts.
test_current:  Receives the calculated current in amperes.

Returns:
    true  = voltage was valid and current was calculated.
    false = output pointer was invalid or voltage was invalid.
*/
bool calculate_test_current(float sense_voltage, float *test_current);


// Calculate resistance using the measured cable voltage and test current.
bool calculate_resistance(float cable_voltage, float test_current, float *resistance_ohms);


// Do a  complete resistance measurement.
// Returns the calculated resistance through resistance_ohms variable.
bool measure_resistance(float *resistance_ohms);


// Perform multiple resistance measurements and return their average.
bool measure_average_resistance(uint8_t sample_count, float *average_resistance_ohms);

// Select one cable conductor and measure its average resistance.
bool measure_conductor_resistance( uint8_t conductor_number, float *resistance_ohms);



/*
 Select one conductor at cable end A and one conductor at
 cable end B, then measure the resistance between them.

 Example:
     test_conductor_pair(2, 5, &resistance);

 tests:
     End A pin 2 -> End B pin 5

 Returns:
     true  = resistance measurement succeeded.
     false = invalid path or measurement failed.
*/
// bool test_conductor_pair(uint8_t end_a_pin, uint8_t end_b_pin, float *resistance_ohms);




typedef enum
{
    PAIR_MEASUREMENT_OK = 0,
    PAIR_INVALID_ARGUMENT,
    PAIR_RELAY_FAULT,
    PAIR_MEASUREMENT_TIMEOUT
} pair_status_t;

typedef struct
{
    bool connected;
    float cable_voltage;
    float sense_voltage;
    float current_a;
    float resistance_ohms; // Meaningful only when connected is true.
} pair_measurement_t;

pair_status_t test_conductor_pair(uint8_t a_pin, uint8_t b_pin, pair_measurement_t *result);

#endif


