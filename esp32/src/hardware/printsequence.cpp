#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "driver/gptimer.h"

#include "projdefines.h"
#include "printsequence.hpp"

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    
    if(gpio_num == PIN_ENDSTOP_R)
    {
      printer.trigRZero();
      esp_rom_printf("Trigger R center\r\n");
    }
    if(gpio_num == PIN_ENDSTOP_FI)
    {
      printer.trigFiZero();
      esp_rom_printf("Trigger Fi center\r\n");
    }
}

static bool IRAM_ATTR rTimer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
  UBaseType_t isrStatus = taskENTER_CRITICAL_FROM_ISR();
  printer.makeRStep();
  taskEXIT_CRITICAL_FROM_ISR(isrStatus);
  return pdFALSE;
}

static bool IRAM_ATTR fiTimer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
  UBaseType_t isrStatus = taskENTER_CRITICAL_FROM_ISR();
  printer.makeFiStep();
  taskEXIT_CRITICAL_FROM_ISR(isrStatus);
  return pdFALSE;
}

void Printer_Init()
{
  printer.initPins(gpio_isr_handler);
  printer.initTimers(rTimer_on_alarm_cb, fiTimer_on_alarm_cb);
  printer.loadSettings();
}