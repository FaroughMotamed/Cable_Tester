#ifndef RELAYS_H
#define RELAYS_H

#include <stdbool.h>  // bool return type.
#include <stdint.h>   // uint8_t and uint16_t parameter types.

bool Relay_Init(void);
bool MCP23017_WriteGPIO(uint8_t board, uint16_t outputs);
bool MCP23017_ReadGPIO (uint8_t board, uint16_t *outputs);

void Relay_AllOff(void);
void Relay_SelectSideA(uint8_t pin);
void Relay_SelectSideB(uint8_t pin);
void Relay_DisconnectSideA(void);
void Relay_DisconnectSideB(void);

#endif /* RELAYS_H */


