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
    
    if(gpio_num == PIN_SECOND_ENDSTOP)
    {
      printer->trigRZero();
      //esp_rom_printf("Trigger R center\r\n");
    }
    if(gpio_num == PIN_FISRT_ENDSTOP)
    {
      printer->trigFiZero();
      //esp_rom_printf("Trigger Fi center\r\n");
    }
}

static bool IRAM_ATTR firstTimer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
  UBaseType_t isrStatus = taskENTER_CRITICAL_FROM_ISR();
  printer->firtsMotorMakeStep();
  taskEXIT_CRITICAL_FROM_ISR(isrStatus);
  portYIELD_FROM_ISR();
  return pdFALSE;
}

static bool IRAM_ATTR secondTimer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
  UBaseType_t isrStatus = taskENTER_CRITICAL_FROM_ISR();
  printer->secondMotorMakeStep();
  taskEXIT_CRITICAL_FROM_ISR(isrStatus);
  portYIELD_FROM_ISR();
  return pdFALSE;
}

void Printer_Init()
{
#ifdef PRINTER_POLAR
  printer = new PolarPrinter;
#else
  printer = new DecartPrinter;
#endif

  printer->initPins(gpio_isr_handler);
  printer->initTimers(firstTimer_on_alarm_cb, secondTimer_on_alarm_cb);
  printer->loadSettings();
}