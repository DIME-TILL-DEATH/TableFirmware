#ifndef PROJDEFINES_H
#define PROJDEFINES_H

// X pads on board - Polar: Fi motor, Decart: Left motor
// Z pads on board - Polar: R motor, Decart: Right motor

#define PRINTER_POLAR //PRINTER_DECART

#define PRIORITY_STATISTIC_TASK 5
#define PRIORITY_TCP_ANSWER_TASK 10
#define PRIORITY_TCP_RECIEVE_TASK 25
#define PRIORITY_FILE_MANAGER_TASK 50
#define PRIORITY_LED_TASK 75
#define PRIORITY_PRINTER_TASK 100
#define PRIORITY_ISR_TIMER_HANDLER 200

// #define GLOBAL_IGNORE_ENDSTOPS
// #define PINS_FREE_JTAG

#endif