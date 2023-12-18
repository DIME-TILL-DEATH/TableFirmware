#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
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
    std::vector<GCode::GAbstractComm*> nextCommBlock = fileManager.readNextBlock();
    if(nextCommBlock.size()>0)
    {
        for(uint16_t cnt=0; cnt<nextCommBlock.size(); cnt++)
        {
          xQueueSendToBack(gcodesQueue, &(nextCommBlock.at(cnt)), pdMS_TO_TICKS(10000));            
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

static bool IRAM_ATTR rTimer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
  printer.makeRStep();
  return pdFALSE;
}

static bool IRAM_ATTR fiTimer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
  printer.makeFiStep();
  return pdFALSE;
}

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

  printer.initPins();
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