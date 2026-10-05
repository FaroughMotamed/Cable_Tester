#ifndef RELAYS_H
#define RELAYS_H

#include <stdbool.h>  // bool return type.
#include <stdint.h>   // uint8_t and uint16_t parameter types.

bool Relay_Init(void);
bool MCP23017_WriteGPIO(uint8_t board, uint16_t outputs);
bool MCP23017_ReadGPIO (uint8_t board, uint16_t *outputs);
bool MCP23017_WritePin(uint8_t board, uint8_t pin, bool on);

bool Relay_AllOff(void);
bool Relay_SelectSideB(uint8_t pin);
bool Relay_DisconnectSideA(void);
bool Relay_DisconnectSideB(void);

#endif /* RELAYS_H */


