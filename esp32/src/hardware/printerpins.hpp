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

        void setFirstMotorStepState(PinState newState);
        void setSecondMotorStepState(PinState newState);
        PinState getFirstMotorStep();
        PinState getFirstMotorDir();
        
        void setFirstMotorDirState(PinState newState);
        void setSecondMotorDirState(PinState newState);
        PinState getSecondMotorStep();
        PinState getSecondMotorDir();
        
    private:
        PinState firstMotorStep{PinState::RESET};
        PinState firstMotorDir{PinState::RESET};

        PinState secondMotorStep{PinState::RESET}; 
        PinState secondMotorDir{PinState::RESET};  

        spi_device_handle_t spiToSR;

        uint8_t srWord{0};
        void srWrite();

        bool polling{false}; // flag for different threads
};

};

#endif