
#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <stdbool.h>
#include <stdint.h>

#define CABLE_MAX_PINS  20U

/*
 Measure the voltage across the cable.
 AIN0 is the positive input-  AIN1 is the negative input.
 cable_voltage receives AIN0 - AIN1 in volts.
 */
// bool measure_cable_voltage(float *cable_voltage);

/*
 Measure the voltage across the 200.0 ohm resistor;
 AIN2 is the positive input- AIN3 is the negative input.
 sense_voltage receives AIN2 - AIN3 in volts.
*/
// bool measure_sense_voltage(float *sense_voltage);


/*
Measure the cable voltage and sense-resistor voltage.

cable_voltage: Receives AIN0 - AIN1 in volts.
sense_voltage: Receives AIN2 - AIN3 in volts.

Returns:
    true  = both voltage measurements succeeded.
    false = an output pointer was invalid or an ADS1220 measurement failed.
*/
// bool measure_voltages(float *cable_voltage, float *sense_voltage);

/*
Calculate the current flowing through the 200-ohm precision sense resistor.

sense_voltage: Measured voltage across the sense resistor in volts.
test_current:  Receives the calculated current in amperes.

Returns:
    true  = voltage was valid and current was calculated.
    false = output pointer was invalid or voltage was invalid.
*/
// bool calculate_test_current(float sense_voltage, float *test_current);


// Calculate resistance using the measured cable voltage and test current.
// bool calculate_resistance(float cable_voltage, float test_current, float *resistance_ohms);


// Do a  complete resistance measurement.
// Returns the calculated resistance through resistance_ohms variable.
// bool measure_resistance(float *resistance_ohms);


// Perform multiple resistance measurements and return their average.
// bool measure_average_resistance(uint8_t sample_count, float *average_resistance_ohms);

// Select one cable conductor and measure its average resistance.
// bool measure_conductor_resistance( uint8_t conductor_number, float *resistance_ohms);



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


typedef struct
{
    uint8_t pin_count;
    uint16_t completed_pairs;

    // Measurements indexed from zero:
    // pair[0][0] represents A1–B1.
    pair_measurement_t pair[CABLE_MAX_PINS][CABLE_MAX_PINS];

    // If a measurement fails, record its status and cable pins.
    pair_status_t fault_status;
    uint8_t fault_a_pin;
    uint8_t fault_b_pin;

    // True only when all final relay-OFF writes succeeded.
    bool relay_shutdown_ok;
} cable_scan_t;




/*
Test one A/B cable-pin pair after selecting relays and waiting 50 ms.

Same-pin checks: two valid samples of sense and cable voltage;
calculate resistance when connected.
Cross checks: one valid sense-voltage sample to detect current;
cable voltage and resistance are not measured.

Retry invalid readings for up to 3 seconds after relay settling.
Accept small negative sense readings within tolerance as zero current.
An open path is a valid result with connected = false.

Return measurement OK, invalid argument, relay fault or timeout.
Leave relays selected on success; attempt all OFF on timeout.
Calibration and cable pass/fail evaluation are handled separately.
*/
pair_status_t test_conductor_pair(uint8_t a_pin, uint8_t b_pin, pair_measurement_t *result);


/*
Scan every connection between Side A and Side B.

For an 8-pin cable, perform 64 checks:
    A1–B1 through A1–B8,
    A2–B1 through A2–B8,
    ...
    A8–B1 through A8–B8.

Sequence:
    1. Validate the result pointer and requested pin count (1–20).
    2. Clear results from the previous scan.
    3. Command all relays OFF. Stop if this fails.
    4. Test each A/B pair and store its measurements.
       Keep Side A selected while stepping through Side B.
       Each pair-selection operation includes 50 ms settling.
    5. Stop early on a relay fault or measurement timeout,
       recording the failed pair and fault status.
    6. Attempt to turn all relays OFF when scanning ends.

Matching-pin checks:
    Collect two valid samples of resistor and cable voltage.
    Calculate resistance when the path is connected.

Cross/short checks:
    Collect one valid sample of resistor voltage only.
    Use the resulting current to detect an unexpected connection.

An open path is a valid measurement and does not stop scanning.

Parameters:
    pin_count -> Number of cable pins to scan, from 1 to 20.
    scan      -> Receives measurements, completed-pair count,
                 fault information and relay-shutdown status.

Returns:
    true  -> Every pair was measured and final OFF commands succeeded.
    false -> Invalid argument, measurement fault or relay-control failure.

IMPORTANT:
    Successful scanning does not mean the cable passed.
    A separate function evaluates connectivity and resistance.
    Successful OFF commands do not prove mechanical contacts opened.
*/
bool scan_cable(uint8_t pin_count, cable_scan_t *scan);


// Final condition of each cable pin.
typedef enum
{
    CONDUCTOR_OK = 0,
    CONDUCTOR_OPEN,
    CONDUCTOR_CROSS,
    CONDUCTOR_SHORT,
    CONDUCTOR_HIGH_RESISTANCE
} conductor_status_t;


typedef struct
{
    conductor_status_t status;

    // Bit 0 means B1, bit 1 means B2, and so on.
    // Records every B pin connected to this A pin.
    uint32_t connected_b_mask;

    // Meaningful when the matching A/B pins are connected.
    float resistance_ohms;
} conductor_result_t;



typedef struct
{
    conductor_status_t status;

    // B pins connected to this A pin.
    // Bit 0 = B1, bit 1 = B2, etc.
    uint32_t connected_b_mask;

    // Other A pins sharing a B connection with this A pin.
    // Bit 0 = A1, bit 1 = A2, etc.
    uint32_t shared_a_mask;

    float resistance_ohms;

} conductor_result_t;

// Interpret a complete scan.
// true means evaluation succeeded; result->passed says whether the cable passed or failed.
bool evaluate_cable_scan(const cable_scan_t *scan, float resistance_limit_ohms, cable_result_t *result);


#endif


