#include "esp_log.h"

#include "printerpins.hpp"

using namespace Pins;

PrinterPins::PrinterPins(gpio_isr_t endStops_cb)
{
    gpio_config_t io_conf = {};

    esp_err_t ret;
    
    spi_bus_config_t buscfg = {};    
    buscfg.mosi_io_num=PIN_SPITOSR_MOSI;
    buscfg.sclk_io_num=PIN_SPITOSR_CLK;
    buscfg.miso_io_num=-1;
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;
    // buscfg.max_transfer_sz=1;

    ret=spi_bus_initialize(VSPI_HOST, &buscfg, SPI_DMA_DISABLED);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg = {};

    devcfg.clock_speed_hz=16*1000*1000;
    devcfg.mode=0;                                
    devcfg.spics_io_num=PIN_SPITOSR_CS;            
    devcfg.queue_size=1;                          

    ret=spi_bus_add_device(VSPI_HOST, &devcfg, &spiToSR);
    ESP_ERROR_CHECK(ret);

    //endstops======================
    io_conf = {};
    uint64_t inputPinsSelection = ((1ULL << PIN_SECOND_ENDSTOP) | (1ULL<<PIN_FISRT_ENDSTOP));
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = inputPinsSelection;
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    //change gpio interrupt type for one pin
    gpio_set_intr_type(PIN_FISRT_ENDSTOP, GPIO_INTR_POSEDGE);

    //install gpio isr service
    uint8_t ESP_INTR_FLAG_DEFAULT=0;
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(PIN_SECOND_ENDSTOP, endStops_cb, (void*) PIN_SECOND_ENDSTOP);
    gpio_isr_handler_add(PIN_FISRT_ENDSTOP, endStops_cb, (void*) PIN_FISRT_ENDSTOP);

    gpio_intr_disable(PIN_SECOND_ENDSTOP);
    gpio_intr_disable(PIN_FISRT_ENDSTOP);
}

void PrinterPins::rStepState(PinState newState)
{
    xMotorStep = newState;
    // gpio_set_level(pinRStep, rStep);
    srWrite();
}

void PrinterPins::fiStepState(PinState newState)
{
    fiStep = newState;
    // gpio_set_level(pinFiStep, fiStep);
    srWrite();
}

void PrinterPins::rDirState(PinState newState)
{
    rDir = newState;
    // gpio_set_level(pinRDir, rDir);
    srWrite();
}

void PrinterPins::fiDirState(PinState newState)
{
    fiDir = newState;
    // gpio_set_level(pinFiDir, fiDir);
    srWrite();
}

PinState PrinterPins::getRStep()
{
    return xMotorStep;
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

    srWord = 0 | (xMotorStep<<1) | (rDir<<2) | (fiStep<<3)  | (fiDir<<4);
               
    t.flags=SPI_TRANS_USE_TXDATA;
    t.length=8;                
    t.tx_data[0] = srWord;               

    if(polling) 
    {
       ESP_LOGE("PRINTER PINS", "WAIT FOR POLLING");
       return;
    }
    polling = true;     
    ret=spi_device_polling_transmit(spiToSR, &t);  
    polling = false;

    if(ret!=ESP_OK)
    {
        ESP_LOGE("PRINTER PINS", "Write request while polling");
    }        
}