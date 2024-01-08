#ifndef PRINTERPINS_H
#define PRINTERPINS_H

#include "pins_esp32_wroom.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

namespace Pins
{

enum PinState
{
    RESET = 0,
    SET = 1  
};

class PrinterPins
{
    public:
        PrinterPins(gpio_isr_t endStops_cb);

        void rStepState(PinState newState);
        void fiStepState(PinState newState);
        void rDirState(PinState newState);
        void fiDirState(PinState newState);

        PinState getRStep();
        PinState getRDir();
        PinState getFiStep();
        PinState getFiDir();
    private:
        // gpio_num_t pinRStep{PIN_RSTEP};
        // gpio_num_t pinRDir{PIN_RDIR};
        // gpio_num_t pinFiStep{PIN_FISTEP};
        // gpio_num_t pinFiDir{PIN_FIDIR};

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