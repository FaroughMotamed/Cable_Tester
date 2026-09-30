


void Relay_AllOff(void)
{

    Board 000:  0000 0000 0000 0000
    Board 001:  0000 0000 0000 0000
    Board 010:  0000 0000 0000 0000

    // Board 000 - Side A
    MCP23017_WriteGPIO(0x00, 0x0000);

    // Board 001 - Side B
    MCP23017_WriteGPIO(0x01, 0x0000);

    // Board 010 - Shared A/B
    MCP23017_WriteGPIO(0x02, 0x0000);
}







