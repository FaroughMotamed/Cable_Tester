

#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include "measurements.h"
#include "cable_mux.h"
#include "ads1220.h"
#include "relays.h"




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


/*
bool test_conductor_pair(uint8_t end_a_pin, uint8_t end_b_pin, float *resistance_ohms)
{
    bool measurement_succeeded;

    // Verify output pointer.
    if (resistance_ohms == NULL)
    {
        return false;
    }

    // Start with a safe output value.
    *resistance_ohms = 0.0f;

    
    // Select the requested path: End A pin -> Force-A + Sense-A -> cable -> Force-B + Sense-B ->  End B pin
    
    if (!cable_mux_select_path(end_a_pin, end_b_pin))
    {
        cable_mux_disable_all();
        return false;
    }

    
     // Measure the resistance of the selected A-to-B path.

     // The MUX function has already allowed the analog
     // path to settle before returning.
    
    measurement_succeeded =  measure_average_resistance(  RESISTANCE_SAMPLES_PER_CONDUCTOR,  resistance_ohms);

    
    // Always disconnect the cable from the measurement circuit after the measurement.
    
    cable_mux_disable_all();

    return measurement_succeeded;
}
*/



/*
Takes two valid samples for matching pins, measuring both voltages.
Takes one valid sample for different pins, measuring only resistor voltage.
Accepts sense voltages from −5 mV to zero as zero current.
Retries invalid measurements and attempts to release all relays on timeout.
Leaves relays selected on success, allowing the scan to keep side A selected.
*/
pair_status_t test_conductor_pair(uint8_t a_pin, uint8_t b_pin, pair_measurement_t *result)
{
    uint32_t start_ms ;
    uint8_t  valid_count = 0U;
    uint8_t  required_count;
    bool     previous_connected = false;

    float cable_voltage = 0.0f;
    float sense_voltage = 0.0f;
    float current;
    float resistance;
    float cable_sum = 0.0f;
    float sense_sum = 0.0f;
    float resistance_sum = 0.0f;

    if (result == NULL)
    {
        return PAIR_INVALID_ARGUMENT;
    }

    *result = (pair_measurement_t){0};


    if ((a_pin < 1U) || (a_pin > CABLE_MAX_PINS) ||
        (b_pin < 1U) || (b_pin > CABLE_MAX_PINS))
    {
        return PAIR_INVALID_ARGUMENT;
    }


    // Select the relays and wait 50 ms inside Relay_SelectPath().
    if (!Relay_SelectPath(a_pin, b_pin))
    {
        return PAIR_RELAY_FAULT;
    }

     if (a_pin == b_pin) { required_count =2U;  }
     else                { required_count = 1U; }

    start_ms = HAL_GetTick();

    while ((HAL_GetTick() - start_ms) < 3000U)
    {

        // Reset values before each measurement attempt.
        cable_voltage = 0.0f;
        sense_voltage = 0.0f;

        // Matching pins need both voltages to calculate cable resistance.
        if (a_pin == b_pin)
        {
            // Existing function reads sense voltage first, cable second.
            if (!measure_voltages(&cable_voltage, &sense_voltage))
            {
                continue;
            }
        }
        else
        {
            // Cross/short checks need only the resistor voltage.
            if (!measure_sense_voltage(&sense_voltage))
            {
                continue;
            }
        }
   

        // Do not accept a sample completed after the deadline.
        if ((HAL_GetTick() - start_ms) >= 3000U)
        {
            break;
        }

        // Reject invalid numbers and unreasonable sense voltages.
        if (!isfinite(cable_voltage) ||
            !isfinite(sense_voltage) ||
            (sense_voltage < -0.010f) ||
            (sense_voltage > 2.5f))
        {
            continue;
        }

        // Small negative readings near zero are accepted as zero current.
        if (sense_voltage < 0.0f)
        {
            sense_voltage = 0.0f;
        }

        current = sense_voltage / 200.0f;

        // Same connectivity threshold used in the relay firmware.
        bool connected = (sense_voltage >= 1.5f);

        resistance = 0.0f;

        // Calculate resistance only for connected matching pins.
        if (connected && (a_pin == b_pin))
        {
            if (!calculate_resistance(cable_voltage, current, &resistance))
            {
                continue;
            }
        }


        // Matching-pin samples must agree about connectivity.
        if ((valid_count > 0U) &&  (connected != previous_connected))
        {
            valid_count = 0U;
            cable_sum = 0.0f;
            sense_sum = 0.0f;
            resistance_sum = 0.0f;
        }

        previous_connected = connected;
        cable_sum += cable_voltage;
        sense_sum += sense_voltage;
        resistance_sum += resistance;
        valid_count++;

        if (valid_count >= required_count)
        {
            result->connected = connected;
            result->cable_voltage = cable_sum / valid_count;
            result->sense_voltage = sense_sum / valid_count;
            result->current_a = result->sense_voltage / 200.0f;
            result->resistance_ohms = resistance_sum / valid_count;

            return PAIR_MEASUREMENT_OK;
        }
    }

    // Measurement fault: attempt to release every relay.
    (void)Relay_AllOff();

    return PAIR_MEASUREMENT_TIMEOUT;
}


/*
Scan all A/B pin pairs and store their measurements.
Keep side A selected while stepping through side B.
Stop on a measurement or relay fault and record the failed pair.
Attempt to turn all relays OFF when the scan ends.
Return true only if the scan and final shutdown succeed;
cable pass/fail is evaluated separately.
*/
bool scan_cable(uint8_t pin_count, cable_scan_t *scan)
{
    uint8_t a_pin;
    uint8_t b_pin;
    pair_status_t status;
    bool measurements_ok = true;
    bool function_output =true;

    if (scan == NULL)
    {
        return false;
    }

    // Clear the measurements and previous fault informationon the enum
    *scan = (cable_scan_t){0};

    if ((pin_count < 1U) || (pin_count > CABLE_MAX_PINS))
    {
        scan->fault_status = PAIR_INVALID_ARGUMENT;
        return false;
    }

    scan->pin_count = pin_count;
    scan->fault_status = PAIR_MEASUREMENT_OK;

    // Begin with every relay commanded OFF.
    if (!Relay_AllOff())
    {
        scan->fault_status = PAIR_RELAY_FAULT;
        return false;
    }

    // Select each Side-A pin in turn.
    for (a_pin = 1U; a_pin <= pin_count; a_pin++)
    {
        // Keep A selected while checking all Side-B pins.
        for (b_pin = 1U; b_pin <= pin_count; b_pin++)
        {
            status = test_conductor_pair(a_pin, b_pin, &scan->pair[a_pin - 1U][b_pin - 1U]);

            if (status != PAIR_MEASUREMENT_OK)
            {
                scan->fault_status = status;
                scan->fault_a_pin = a_pin;
                scan->fault_b_pin = b_pin;

                measurements_ok = false;
                break;
            }

            scan->completed_pairs++;
        }

        // Stop the outer loop too if a measurement failed.
        if (!measurements_ok)
        {
            break;
        }
    }

    // Always attempt shutdown after starting the scan.
    scan->relay_shutdown_ok = Relay_AllOff();

    function_output = measurements_ok && scan->relay_shutdown_ok;

    return function_output;
}




/*
Interpret a completed cable scan.

For each A pin:
    - No connected B pins: OPEN.
    - Multiple connections involving that path: SHORT.
    - One connection to the wrong B pin: CROSS.
    - Matching connection above the resistance limit: HIGH RESISTANCE.
    - Otherwise: OK.

A cable passes only when every selected pin is OK.
*/
bool evaluate_cable_scan(const cable_scan_t *scan, float resistance_limit_ohms, cable_result_t *result)
{
    uint8_t row_count[CABLE_MAX_PINS] = {0};
    uint8_t column_count[CABLE_MAX_PINS] = {0};

    uint8_t a;
    uint8_t b;
    uint8_t pin_count;
    bool short_detected;
    float resistance;

    if (result == NULL)
    {
        return false;
    }

    *result = (cable_result_t){0};

    if ((scan == NULL) || !isfinite(resistance_limit_ohms) || (resistance_limit_ohms <= 0.0f))
    {
        return false;
    }

    pin_count = scan->pin_count;

    // if there is a wrong condition, return false and don't go to complete evaluation
    if ((pin_count < 1U) ||
        (pin_count > CABLE_MAX_PINS) ||
        (scan->fault_status != PAIR_MEASUREMENT_OK) ||
        !scan->relay_shutdown_ok ||
        (scan->completed_pairs !=
         (uint16_t)(pin_count * pin_count)))
    {
        return false;
    }

    /*
    Count connections in both directions.
    if there is a onnection batween pin a on A side and pon b on B side,
    increment both pin a and pin b. Pins where the count is more than 1,
    means there is two connections to them.

    Checking columns also detects two different A pins
    connected to the same B pin.
    */
    for (a = 0U; a < pin_count; a++)
    {
        for (b = 0U; b < pin_count; b++)
        {
            if (scan->pair[a][b].connected)
            {
                row_count[a] = row_count[a] +1;
                column_count[b] = column_count[b] +1;
            }
        }
    }

    // Verify resistance values before producing final results.
    for (a = 0U; a < pin_count; a++)
    {
        if (scan->pair[a][a].connected)
        {
            resistance = scan->pair[a][a].resistance_ohms;

            if (!isfinite(resistance) || (resistance < 0.0f))
            {
                return false;
            }
        }
    }

    result->pin_count = pin_count;
    result->passed = true;

    for (a = 0U; a < pin_count; a++)
    {

        // Save B destinations and other A pins sharing those destinations.
        for (b = 0U; b < pin_count; b++)
        {
            if (scan->pair[a][b].connected)
            {
                // Record this B destination.
                result->conductor[a].connected_b_mask |= ((uint32_t)1U << b);

                // Find other A pins connected to the same B pin.
                for (uint8_t other = 0U; other < pin_count; other++)
                {
                    if ((other != a) &&
                        scan->pair[other][b].connected)
                    {
                        result->conductor[a].shared_a_mask |= ((uint32_t)1U << other);
                    }
                }
            }
        }

        if (scan->pair[a][a].connected)
        {
            result->conductor[a].resistance_ohms = scan->pair[a][a].resistance_ohms;
        }

        if (row_count[a] == 0U)
        {
            result->conductor[a].status = CONDUCTOR_OPEN;
        }
        else
        {
            short_detected = (row_count[a] > 1U);

            // A single B destination may also be shared by other A pins.
            for (b = 0U; b < pin_count; b++)
            {
                if (scan->pair[a][b].connected &&
                    (column_count[b] > 1U))
                {
                    short_detected = true;
                }
            }

            if (short_detected)
            {
                result->conductor[a].status = CONDUCTOR_SHORT;
            }
            else if (!scan->pair[a][a].connected)
            {
                result->conductor[a].status = CONDUCTOR_CROSS;
            }
            else if (result->conductor[a].resistance_ohms >  resistance_limit_ohms)
            {
                result->conductor[a].status = CONDUCTOR_HIGH_RESISTANCE;
            }
            else
            {
                result->conductor[a].status = CONDUCTOR_OK;
            }
        }

        if (result->conductor[a].status != CONDUCTOR_OK)
        {
            result->passed = false;
        }
    }

    return true;
}









