#ifndef PRINTERPINS_H
#define PRINTERPINS_H

#include "driver/gpio.h"
#include "driver/spi_master.h"

namespace Pins
{

enum PinState
{
    SET = 0,
    RESET
};

class PrinterPins
{
    public:
        PrinterPins();

        void rStepState(PinState newState);
        void fiStepState(PinState newState);
        void rDirState(PinState newState);
        void fiDirState(PinState newState);

        PinState getRStep();
        PinState getRDir();
        PinState getFiStep();
        PinState getFiDir();
    private:
        gpio_num_t pinRStep{GPIO_NUM_19};
        gpio_num_t pinRDir{GPIO_NUM_25};
        gpio_num_t pinFiStep{GPIO_NUM_23};
        gpio_num_t pinFiDir{GPIO_NUM_26};

        PinState rStep{PinState::RESET};
        PinState fiStep{PinState::RESET};
        PinState rDir{PinState::RESET};
        PinState fiDir{PinState::RESET};

        spi_device_handle_t spiToSR;

        uint8_t srWord{0};
        void srWrite();
};

};

#endif