
/*=================================================================================*/
//************* Overall description of what different files should do  *************
/*
cable_tester.c/.h     Overall state machine and test sequence
adg732.c/.h           MUX control
ads1220.c/.h          Precision ADC communication
selector.c/.h         Six-position selector
encoder.c/.h          Rotary encoder
calibration.c/.h      1 Ω and 5 Ω calibration
indicators.c/.h       LEDs and buzzer
*/

/*=================================================================================*/
//************* Steps of what the cable_tester_init() function should do ************
// cable_tester_init()
/*
1. Enter safe state
2. Initialize ADS1220 (precision ADC, Analoge Digital Converter )
3. Load calibration from Flash
4. Initialize  encoder
5. Read the selector
6. Initialize  display
7. Verify current source
8. Enter READY state
*/

/*=================================================================================*/
//*************  Steps enter_safe_sate() fuction sohuld do at start up *************
// safe state means all the periphals should be in a safe state before starting the measurements.

/*
1. Disable four ADG732 MUXes                     --> individual  MUX /EN signals  = HIGH
2. Deselect four MUX programming interfaces      --> individual  MUX /CS signals  = HIGH
3. Make the shared MUX write signal inactive     --> shared      MUX /WR          = HIGH
4. Set the shared MUX address to all zero        --> shared      MUX A4–A0        = 00000
5. Deselect  ADS1220 precision ADC               -->             ADS1220 /CS      = HIGH
6. Deselect  display SPI devices                 -->             LCD /CS   = HIGH, Flash /CS = HIGH
7. Place LCD control signals in known states     -->             LCD reset = HIGH, LCD DCX = LOW
8. Turn off  3 indicator LEDs                    -->             Green PASS LED = LOW, Red FAIL LED = LOW , Blue STATUS LED = LOW
9. Turn off the buzzer                           -->             Buzzer = LOW
10. Reset state flags 
*/

/// Summary:
/*
MUX /EN signals       = HIGH
MUX /CS signals       = HIGH
MUX /WR               = HIGH
MUX A4–A0             = 00000
ADS1220 /CS           = HIGH
LCD /CS               = HIGH
Flash /CS             = HIGH
LCD reset             = HIGH
LCD DCX               = LOW
Red FAIL LED          = LOW
Blue STATUS LED       = LOW
Green PASS LED        = LOW
Buzzer                = LOW
*/


#ifndef CABLE_TESTER_H
#define CABLE_TESTER_H

void enter_safe_state(void);

//void cable_tester_init(void);
//void cable_tester_process(void);

#endif

































