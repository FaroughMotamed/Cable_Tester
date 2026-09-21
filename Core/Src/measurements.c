

#include "measurements.h"
#include "ads1220.h"
#include <stddef.h>


// Precision sense resistor used by the current source.
#define SENSE_RESISTOR_OHMS             200.0f

// Maximum reasonable voltage across the sense resistor.
#define SENSE_VOLTAGE_MAXIMUM_V         2.5f


// The current is about 10.24 mA. The below threshold is only to prevetn division be zero.
#define MINIMUM_TEST_CURRENT_A       0.001f

// Highest resistance accepted by this device.
// The theoretical calculation is around 100 ohm.
#define MAXIMUM_RESISTANCE_OHMS      200.0f


/*
 Measure the voltage across the cable.
 AIN0 is the positive input - AIN1 is the negative input.
 cable_voltage receives AIN0 - AIN1 in volts.
 */
bool measure_cable_voltage(float *cable_voltage){

  bool output= ads1220_measure_input(ADS1220_CABLE_SELECT, cable_voltage);
  return output;
}



/*
 Measure the voltage across the 200.0 ohm resistor;
 AIN2 is the positive input - AIN3 is the negative input.
 sense_voltage receives AIN2 - AIN3 in volts.
*/
bool measure_sense_voltage(float *sense_voltage){

  bool output= ads1220_measure_input(ADS1220_RESISTOR_SELECT, sense_voltage);
  return output;
}


/*
Measure both voltages required to calculate cable resistance.

The sense-resistor voltage is measured first so the firmware
can later verify that the expected test current is flowing.
The cable voltage is measured second.
Returns true only when both measurements succeed.
*/
bool measure_voltages(float *cable_voltage, float *sense_voltage)
{
    // Both output pointers must be valid.
    if ((cable_voltage == NULL) || (sense_voltage == NULL))
    {
        return false;
    }

    // Start with safe known output values.
    *cable_voltage = 0.0f;
    *sense_voltage = 0.0f;

 
    // Measure AIN2 - AIN3 across the 200-ohm precision sense resistor.
    if (!measure_sense_voltage(sense_voltage))
    {
        return false;
    }

 
    // Measure AIN0 - AIN1 across the selected cable conductor.
    if (!measure_cable_voltage(cable_voltage))
    {
        return false;
    }

    // Both measurements completed successfully.
    return true;
}



/*
Calculate test current using Ohm's law:
    current = voltage / resistance

For this cable tester:
    resistance = 200 ohms
    sense voltage = 2.048 V
    current = 2.048 V / 200 ohms = 0.01024 A = 10.24 mA
*/
bool calculate_test_current(float sense_voltage, float *test_current)
{
    // The caller must provide a valid output address.
    if (test_current == NULL)
    {
        return false;
    }

    // Begin with a safe output value.
    *test_current = 0.0f;

    /*
    A negative sense voltage indicates reversed measurement 
    polarity, incorrect wiring or an invalid measurement.
    */
    if (sense_voltage < 0.0f)
    {
        return false;
    }

    /*
    A voltage above the expected maximum indicates an
    invalid measurement or current-source problem.
    */
    if (sense_voltage > SENSE_VOLTAGE_MAXIMUM_V)
    {
        return false;
    }

    // Calculate current in amperes using Ohm's law.
    *test_current = sense_voltage / SENSE_RESISTOR_OHMS;

    return true;
}

// Apply Ohm's law: R = V / I for the cable.
bool calculate_resistance(float cable_voltage, float test_current, float *resistance_ohms)
{
    float calculated_resistance;

    // Verify that the caller provided a valid output address.
    if (resistance_ohms == NULL)
    {
        return false;
    }

    // Start with a safe output value.
    *resistance_ohms = 0.0f;

    // A negative cable voltage is not valid for this circuit.
    if (cable_voltage < 0.0f)
    {
        return false;
    }

    /*
    Do not divide by zero or by a very small current.

    A very small current normally means that the cable is open,
    the current source is not operating, or the measurement is invalid.
    */
    if (test_current < MINIMUM_TEST_CURRENT_A)
    {
        return false;
    }

    // Apply Ohm's law: R = V / I.
    calculated_resistance = cable_voltage / test_current;

    // Reject a result outside the supported resistance range.
    if (calculated_resistance > MAXIMUM_RESISTANCE_OHMS)
    {
        return false;
    }

    // Copy the valid result to the caller.
    *resistance_ohms = calculated_resistance;

    return true;
}


