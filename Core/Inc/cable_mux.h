

#ifndef CABLE_MUX_H
#define CABLE_MUX_H

#include <stdbool.h>
#include <stdint.h>

// Disconnect every cable conductor from the measurement circuit.
void cable_mux_disable_all(void);

// Select one cable conductor on all measurement MUXes.
// conductor_number uses the user-facing range 1 through 8.
bool cable_mux_select_conductor(uint8_t conductor_number);

#endif