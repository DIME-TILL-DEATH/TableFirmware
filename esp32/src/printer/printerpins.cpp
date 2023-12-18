#include "esp_log.h"

#include "printerpins.hpp"

using namespace Pins;

PrinterPins::PrinterPins()
{
    uint64_t outputPinsSelection = ((1ULL << pinRStep) | (1ULL<<pinRDir) | (1ULL<<pinFiStep) | (1ULL<<pinFiDir));

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT; 
    io_conf.pin_bit_mask = outputPinsSelection;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;  
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   
    gpio_config(&io_conf);   

    gpio_set_level(pinRStep, 0);
    gpio_set_level(pinRDir, 0);
    gpio_set_level(pinFiStep, 0);
    gpio_set_level(pinFiDir, 0);

    ESP_LOGI("PRINTER PINS:", "pinRstep: %d, pinRDir: %d, pinFiStep: %d, pinFiDir: %d\r\n", pinRStep, pinRDir, pinFiStep, pinFiDir);
}

void PrinterPins::rStepState(PinState newState)
{
    rStep = newState;
    gpio_set_level(pinRStep, rStep);
}

void PrinterPins::fiStepState(PinState newState)
{
    fiStep = newState;
    gpio_set_level(pinFiStep, fiStep);
}

void PrinterPins::rDirState(PinState newState)
{
    rDir = newState;
    gpio_set_level(pinRDir, rDir);
}

void PrinterPins::fiDirState(PinState newState)
{
    fiDir = newState;
    gpio_set_level(pinFiDir, fiDir);
}

PinState PrinterPins::getRStep()
{
    return rStep;
}

PinState PrinterPins::getFiStep()
{
    return fiStep;
}

PinState PrinterPins::getRDir()
{
    return rDir;
}

PinState PrinterPins::getFiDir()
{
    return fiDir;
}