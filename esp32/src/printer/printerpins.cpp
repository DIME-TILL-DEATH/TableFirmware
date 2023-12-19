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

    esp_err_t ret;
    
    spi_bus_config_t buscfg = {};    
    buscfg.mosi_io_num=21;
    buscfg.sclk_io_num=16;
    buscfg.miso_io_num=-1;
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;
    // buscfg.max_transfer_sz=1;

    ret=spi_bus_initialize(VSPI_HOST, &buscfg, SPI_DMA_DISABLED);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg = {};

    devcfg.clock_speed_hz=16*1000*1000;
    devcfg.mode=0;                                
    devcfg.spics_io_num=17;            
    devcfg.queue_size=1;                          

    ret=spi_bus_add_device(VSPI_HOST, &devcfg, &spiToSR);
    ESP_ERROR_CHECK(ret);
}

void PrinterPins::rStepState(PinState newState)
{
    rStep = newState;
    gpio_set_level(pinRStep, rStep);
    srWrite();
}

void PrinterPins::fiStepState(PinState newState)
{
    fiStep = newState;
    gpio_set_level(pinFiStep, fiStep);
    srWrite();
}

void PrinterPins::rDirState(PinState newState)
{
    rDir = newState;
    gpio_set_level(pinRDir, rDir);
    srWrite();
}

void PrinterPins::fiDirState(PinState newState)
{
    fiDir = newState;
    gpio_set_level(pinFiDir, fiDir);
    srWrite();
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

void PrinterPins::srWrite()
{
    esp_err_t ret;
    spi_transaction_t t = {};

    srWord = 1 | (rStep<<1) | (rDir<<2) | (fiStep<<3)  | (fiDir<<4);
               
    t.flags=SPI_TRANS_USE_TXDATA;
    t.length=8;                
    t.tx_data[0] = srWord;               
            
    ret=spi_device_polling_transmit(spiToSR, &t);  
    assert(ret==ESP_OK);            
}