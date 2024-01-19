#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "net/wifi.h"
#include "net/tcpip.hpp"

#include "projdefines.h"
#include "printer/printsequence.hpp"
#include "printer/printertask.hpp"
#include "filemanager/filetask.hpp"
#include "netcomm/abstractcommand.hpp"

#define PRINTER_COMM_QUEUE_SIZE 32
#define NET_COMM_QUEUE_SIZE 32

QueueHandle_t gcodesQueue, printReqQueue, fileReqQueue, netAnswQueue;

extern "C" void app_main(void)
{
  gcodesQueue = xQueueCreate(PRINTER_COMM_QUEUE_SIZE, sizeof(GCode::GAbstractComm*));
  printReqQueue = xQueueCreate(NET_COMM_QUEUE_SIZE, sizeof(NetComm::AbstractCommand*));
  fileReqQueue = xQueueCreate(NET_COMM_QUEUE_SIZE, sizeof(NetComm::AbstractCommand*));
  netAnswQueue = xQueueCreate(NET_COMM_QUEUE_SIZE, sizeof(NetComm::AbstractCommand*));

  WIFI_Init();
  TCPIP_Init();
  Printer_Init();

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