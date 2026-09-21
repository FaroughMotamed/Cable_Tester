

#include <stddef.h>
#include <stdint.h>
#include "measurements.h"
#include "cable_mux.h"
#include "ads1220.h"


// Precision sense resistor used by the current source.
#define SENSE_RESISTOR_OHMS             200.0f

// Maximum reasonable voltage across the sense resistor.
#define SENSE_VOLTAGE_MAXIMUM_V         2.5f


// The current is about 10.24 mA. The below threshold is only to prevetn division be zero.
#define MINIMUM_TEST_CURRENT_A          0.001f

// Highest resistance accepted by this device.
// The theoretical calculation is around 100 ohm.
#define MAXIMUM_RESISTANCE_OHMS         200.0f

#define RESISTANCE_SAMPLES_PER_CONDUCTOR  4U


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
    prevent division by zero or by a very small current.

    A very small current means that the cable is open,
    the current source is not operating, or the measurement is invalid.
    */
    if (test_current < MINIMUM_TEST_CURRENT_A)
    {
        return false;
    }

    // R = V / I.
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


// Do a measure_resistance
bool measure_resistance(float *resistance_ohms)
{
    float cable_voltage;
    float sense_voltage;
    float test_current;
    float calculated_resistance;

    // verify that the caller provided a valid output address.
    if (resistance_ohms == NULL)
    {
        return false;
    }

    // Start with a safe output value.
    *resistance_ohms = 0.0f;

    /*
    Measure both ADS1220 input pairs:

    AIN0 - AIN1 measures the  cable voltage.
    AIN2 - AIN3 measures the voltage across the 200-ohm  resistor.
    */
    if (!measure_voltages(&cable_voltage, &sense_voltage))
    {
        return false;
    }

    // Calculate the test current from the sense-resistor voltage.
    if (!calculate_test_current(sense_voltage, &test_current))
    {
        return false;
    }

    // Apply Ohm's law to calculate the DUT resistance.
    if (!calculate_resistance(cable_voltage, test_current, &calculated_resistance))
    {
        return false;
    }

    // Copy the valid resistance to the caller.
    *resistance_ohms = calculated_resistance;

    return true;
}

/*
64 × 22.27 ms = 1425 ms conversion time

1425 ms
+ 64 ms ADS settling delays
+ 16 ms cable MUX settling
+ approximately 50–150 ms overhead
~= 1.56–1.66 seconds
*/

bool measure_average_resistance(uint8_t sample_count, float *average_resistance_ohms)
{
    float resistance_sum = 0.0f;
    float measured_resistance;
    uint8_t sample_index;
    uint8_t valid_sample_count = 0U;

    // Verify that the caller provided a valid output address.
    if (average_resistance_ohms == NULL)
    {
        return false;
    }

    // Start with a safe output value.
    *average_resistance_ohms = 0.0f;

    // Attempt the requested number of resistance measurements.
    for (sample_index = 0U; sample_index < sample_count; sample_index++)
    {
        if (measure_resistance(&measured_resistance))
        {
            // Include only successful measurements in the sum.
            resistance_sum = resistance_sum + measured_resistance;
            valid_sample_count++;
        }
    }

    // An average cannot be calculated if every sample failed.
    if (valid_sample_count == 0U)
    {
        return false;
    }

    // Divide by the number of successful samples, not by the number of requested samples.
    *average_resistance_ohms = resistance_sum / (float)valid_sample_count;

    return true;
}


// Select one cable conductor and measure its average resistance.
bool measure_conductor_resistance( uint8_t conductor_number, float *resistance_ohms)
{
    bool measurement_succeeded;

    // Verify that the caller provided a valid output address.
    if (resistance_ohms == NULL)
    {
        return false;
    }

    // Start with a safe output value.
    *resistance_ohms = 0.0f;

    /*
    Configure the cable MUXes so that the requested conductor
    is connected to the current source and measurement circuit.

    cable_mux_select_conductor() also allows the analog path  to settle before returning.*/
    if (!cable_mux_select_conductor(conductor_number))
    {
        cable_mux_disable_all();
        return false;
    }

    /* Attempt four resistance measurements.
       This function fails only if all four samples fail. */
    measurement_succeeded = measure_average_resistance( RESISTANCE_SAMPLES_PER_CONDUCTOR, resistance_ohms);

    /* Disconnect the cable from the measurement circuit after
       the measurement, regardless of whether it succeeded. */
    cable_mux_disable_all();

    return measurement_succeeded;
}




