#ifndef PINS_ESP32_WR0OM_H
#define PINS_ESP32_WR0OM_H

#include "driver/gpio.h"

#include "projdefines.h"

#ifdef PINS_FREE_JTAG
#define PIN_SDCARD_MISO GPIO_NUM_23 //LCD_MOSI
#define PIN_SDCARD_MOSI GPIO_NUM_19 //LCD_MISO
#define PIN_SDCARD_CLK  GPIO_NUM_18 //LCD_SCK
#define PIN_SDCARD_CS   GPIO_NUM_5 //LCD_EN_O

#else
#define PIN_SDCARD_MISO GPIO_NUM_12
#define PIN_SDCARD_MOSI GPIO_NUM_13
#define PIN_SDCARD_CLK  GPIO_NUM_14
#define PIN_SDCARD_CS   GPIO_NUM_15
#endif

#define PIN_SPITOSR_MOSI GPIO_NUM_21
#define PIN_SPITOSR_CLK GPIO_NUM_16
#define PIN_SPITOSR_CS GPIO_NUM_17

#define PIN_FISRT_ENDSTOP GPIO_NUM_34  //Z pins on board, Polar: Fi, Decart: X coordinates
#define PIN_SECOND_ENDSTOP GPIO_NUM_35   //Y pins on board, Polar: R, Decart: Y coordinates


#endif