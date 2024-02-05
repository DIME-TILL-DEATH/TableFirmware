#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_app_desc.h"

#include "net/wifi.hpp"
#include "net/tcpip.hpp"

#include "projdefines.h"
#include "firmware.hpp"

#include "printer/printsequence.hpp"
#include "printer/printertask.hpp"
#include "filemanager/filetask.hpp"
#include "netcomm/abstractcommand.hpp"

#define PRINTER_COMM_QUEUE_SIZE 32
#define NET_COMM_QUEUE_SIZE 32

QueueHandle_t gcodesQueue, printReqQueue, fileReqQueue, netAnswQueue;

FileManager fileManager;

extern "C" void app_main(void)
{
  FW_StartupCheck();

  WIFI_Init();
  TCPIP_Init();

  gcodesQueue = xQueueCreate(PRINTER_COMM_QUEUE_SIZE, sizeof(GCode::GAbstractComm*));
  printReqQueue = xQueueCreate(NET_COMM_QUEUE_SIZE, sizeof(NetComm::AbstractCommand*));
  fileReqQueue = xQueueCreate(NET_COMM_QUEUE_SIZE, sizeof(NetComm::AbstractCommand*));
  netAnswQueue = xQueueCreate(NET_COMM_QUEUE_SIZE, sizeof(NetComm::AbstractCommand*));



  while(fileManager.connectSDCard() != FM_OK)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  if(gcodesQueue != NULL)
  {
    xTaskCreatePinnedToCore(file_task, "file_manager", 4096, NULL, PRIORITY_FILE_MANAGER_TASK, NULL, 1);
    xTaskCreatePinnedToCore(printer_task, "printer", 4096, NULL, PRIORITY_PRINTER_TASK, NULL, 1);
  }
  else
  {
    ESP_LOGE("MAIN", "Failed to create gCodes queue");
  }
}