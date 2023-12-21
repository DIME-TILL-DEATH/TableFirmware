#ifndef PINS_ESP32_WR0OM_H
#define PINS_ESP32_WR0OM_H

#include "driver/gpio.h"

#define PIN_SDCARD_MISO GPIO_NUM_12
#define PIN_SDCARD_MOSI GPIO_NUM_13
#define PIN_SDCARD_CLK  GPIO_NUM_14
#define PIN_SDCARD_CS   GPIO_NUM_15

#define PIN_RSTEP GPIO_NUM_19
#define PIN_RDIR GPIO_NUM_25
#define PIN_FISTEP GPIO_NUM_23
#define PIN_FIDIR GPIO_NUM_26

#define PIN_SPITOSR_MOSI GPIO_NUM_21
#define PIN_SPITOSR_CLK GPIO_NUM_16
#define PIN_SPITOSR_CS GPIO_NUM_17

#define PIN_ENDSTOP_R GPIO_NUM_35   //Y
#define PIN_ENDSTOP_FI GPIO_NUM_34  //Z

#endif