

#ifndef CABLE_MUX_H
#define CABLE_MUX_H

#include <stdbool.h>
#include <stdint.h>


typedef enum
{
    CABLE_MUX_END_A_FORCE = 0,
    CABLE_MUX_END_A_SENSE,
    CABLE_MUX_END_B_FORCE,
    CABLE_MUX_END_B_SENSE
} cable_mux_id_t;

// Disconnect every cable conductor from the measurement circuit.
void cable_mux_disable_all(void);

// Select one cable conductor on all measurement MUXes.
// conductor_number uses the user-facing range 1 through 8.
bool cable_mux_select_conductor(uint8_t conductor_number);


// Connect one conductor at cable end A to one conductor at cable end B.
bool cable_mux_select_path(uint8_t end_a_conductor,  uint8_t end_b_conductor);


// Store a channel selection in one ADG732.
// The channel number uses the physical range S1 through S32
bool cable_mux_set_channel(cable_mux_id_t mux,  uint8_t conductor_number);

void cable_mux_enable(cable_mux_id_t mux);

void cable_mux_disable(cable_mux_id_t mux);

void cable_mux_disable_all(void);



#endif