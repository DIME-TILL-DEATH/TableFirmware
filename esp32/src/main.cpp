#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_app_desc.h"

#include "esp_efuse.h"
#include "esp_efuse_table.h"

#include "net/wifi.hpp"
#include "net/tcpip.hpp"

#include "projdefines.h"
#include "firmware.hpp"

#include "hardware/printsequence.hpp"
#include "hardware/hwtask.hpp"
#include "filemanager/filetask.hpp"

#include "messages/abstractmessage.h"

#include "esp_task_wdt.h"

#define PRINTER_COMM_QUEUE_SIZE 32
#define NET_COMM_QUEUE_SIZE 16

QueueHandle_t gcodesQueue, printReqQueue, fileReqQueue, netAnswQueue;
SemaphoreHandle_t spiMutex;

FileManager fileManager;

TaskHandle_t fileTaskHandle;
TaskHandle_t hwTaskhandle;

extern "C" void app_main(void)
{
  FW_StartupCheck();

  // WIFI_Init();
  // TCPIP_Init();

  gcodesQueue = xQueueCreate(PRINTER_COMM_QUEUE_SIZE, sizeof(GCode::GAbstractComm*));
  printReqQueue = xQueueCreate(NET_COMM_QUEUE_SIZE, sizeof(AbstractMessage*));
  fileReqQueue = xQueueCreate(NET_COMM_QUEUE_SIZE, sizeof(AbstractMessage*));
  netAnswQueue = xQueueCreate(NET_COMM_QUEUE_SIZE, sizeof(AbstractMessage*));

  spiMutex = xSemaphoreCreateMutex();

  while(fileManager.connectSDCard() != FM_OK)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  vTaskDelay(pdMS_TO_TICKS(100));

  WIFI_Init();
  TCPIP_Init();

  if(gcodesQueue != NULL)
  {
    xTaskCreatePinnedToCore(file_task, "file_manager", 1024*8, NULL, PRIORITY_FILE_MANAGER_TASK, &fileTaskHandle, 1);
    xTaskCreatePinnedToCore(hardware_task, "printer", 4096, NULL, PRIORITY_PRINTER_TASK, &hwTaskhandle, 1);
  }
  else
  {
    ESP_LOGE("MAIN", "Failed to create gCodes queue");
  }
}