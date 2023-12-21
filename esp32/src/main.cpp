#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/gptimer.h"

#include "net/wifi.h"
#include "filemanager/filemanager.hpp"
#include "printer/printer.hpp"

FileManager fileManager;
Printer printer;

#define PRIORITY_FILE_MANAGER_TASK 50
#define PRIORITY_PRINTER_TASK 100
#define PRIORITY_ISR_TIMER_HANDLER 200

static portMUX_TYPE printMutex;
static SemaphoreHandle_t rTimerSemaphore, fiTimerSemaphore;
QueueHandle_t gcodesQueue;

static void fileManager_task(void *arg)
{
  FM_RESULT result;
  do
  {
    result = fileManager.connectSDCard();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }while(result != FM_OK);

  fileManager.loadPlaylist();

  for(;;)
  {
    // if(uxQueueSpacesAvailable(gcodesQueue))
    // {
      std::vector<GCode::GAbstractComm*> nextCommBlock = fileManager.readNextBlock();
      if(nextCommBlock.size()>0)
      {
          for(uint16_t cnt=0; cnt<nextCommBlock.size(); cnt++)
          {
            xQueueSendToBack(gcodesQueue, &(nextCommBlock.at(cnt)), portMAX_DELAY);            
            taskYIELD();
          }
      }
      else
      {
          do
          {
              result = fileManager.loadNextPrint();
          }
          while(result != FM_OK);
      } 
    }
  // }
}

static void printer_task(void *arg)
{
  for(;;)
  { 
    if(!printer.isQueueFull())
    {
      GCode::GAbstractComm* recComm;
      portBASE_TYPE xStatus = xQueueReceive(gcodesQueue, &recComm, pdMS_TO_TICKS(0));
      if(xStatus == pdPASS)
      {
         printer.pushPrintCommand(recComm);
      }
    }

    printer.printRoutine();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

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

// ISR========================================
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
// MAIN=========================================

extern "C" void app_main(void)
{
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  WIFI_Init();

  printMutex = portMUX_INITIALIZER_UNLOCKED;

  rTimerSemaphore = xSemaphoreCreateBinary();
  fiTimerSemaphore = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(rTimer_task, "rTimer", 1024, NULL, PRIORITY_ISR_TIMER_HANDLER, &rTimer_taskHandle, 1);
  xTaskCreatePinnedToCore(fiTimer_task, "fiTimer", 1024, NULL, PRIORITY_ISR_TIMER_HANDLER, &fiTimer_taskHandle, 1);

  printer.initPins(gpio_isr_handler);
  printer.initTimers(rTimer_on_alarm_cb, fiTimer_on_alarm_cb);

  gcodesQueue = xQueueCreate(FileManager::blockSize, sizeof(GCode::GAbstractComm*));
  if(gcodesQueue != NULL)
  {
    xTaskCreatePinnedToCore(fileManager_task, "file_manager", 4096, NULL, PRIORITY_FILE_MANAGER_TASK, NULL, 1);
    xTaskCreatePinnedToCore(printer_task, "printer", 4096, NULL, PRIORITY_PRINTER_TASK, NULL, 1);
  }
  else
  {
    ESP_LOGE("MAIN", "Failed to create gCodes queue");
  }
}

