
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

#endif


