#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "driver/gptimer.h"

#include "projdefines.h"
#include "printsequence.hpp"

static portMUX_TYPE printMutex;
static SemaphoreHandle_t rTimerSemaphore, fiTimerSemaphore;

TaskHandle_t rTimer_taskHandle;
static void rTimer_task(void *arg)
{
  for( ;; )
  {
    if(xSemaphoreTake(rTimerSemaphore, portMAX_DELAY))
    {
      taskENTER_CRITICAL(&printMutex);
      printer.makeRStep();
      taskEXIT_CRITICAL(&printMutex);
    }
  }
}

TaskHandle_t fiTimer_taskHandle;
static void fiTimer_task(void *arg)
{
  for( ;; )
  {
    if(xSemaphoreTake(fiTimerSemaphore, portMAX_DELAY))
    {
      taskENTER_CRITICAL(&printMutex);
      printer.makeFiStep(); 
      taskEXIT_CRITICAL(&printMutex);
    }
  }
}

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
  xSemaphoreGiveFromISR(rTimerSemaphore, NULL);
  return xTaskResumeFromISR(rTimer_taskHandle);
}

static bool IRAM_ATTR fiTimer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
  xSemaphoreGiveFromISR(fiTimerSemaphore, NULL);
  return xTaskResumeFromISR(fiTimer_taskHandle);
}

void Printer_Init()
{
  printMutex = portMUX_INITIALIZER_UNLOCKED;

  rTimerSemaphore = xSemaphoreCreateBinary();
  fiTimerSemaphore = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(rTimer_task, "rTimer", 1024, NULL, PRIORITY_ISR_TIMER_HANDLER, &rTimer_taskHandle, 1);
  xTaskCreatePinnedToCore(fiTimer_task, "fiTimer", 1024, NULL, PRIORITY_ISR_TIMER_HANDLER, &fiTimer_taskHandle, 1);

  printer.initPins(gpio_isr_handler);
  printer.initTimers(rTimer_on_alarm_cb, fiTimer_on_alarm_cb);
  printer.loadSettings();
}