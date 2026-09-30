


void Relay_AllOff(void)
{
    /*
    Board 000:  0000 0000 0000 0000
    Board 001:  0000 0000 0000 0000
    Board 010:  0000 0000 0000 0000
    */

    // Board 000 - Side A
    MCP23017_WriteGPIO(0x00, 0x0000);

    // Board 001 - Side B
    MCP23017_WriteGPIO(0x01, 0x0000);

    // Board 010 - Shared A/B
    MCP23017_WriteGPIO(0x02, 0x0000);
}

void Relay_SelectSideA(uint8_t pin)
{
    // First disconnect all Side-A relays
    Relay_DisconnectSideA();

    /*
     Select the requested Side-A cable pin.
     Pins handled by relay board 000 come first.
     Remaining Side-A pins use channels 1-4 of shared relay board 010.
     */

    if (pin >= 1 && pin <= 16)
    {
        /* Board 000 */
        MCP23017_WritePin(0x00, pin - 1, 1);
    }
    else if (pin >= 17 && pin <= 20)
    {
        /* Shared board 010, channels 1-4 */
        MCP23017_WritePin(0x02, pin - 17, 1);
    }
}





