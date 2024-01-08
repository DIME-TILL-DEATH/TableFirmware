#ifndef PRINTERTASK_H
#define PRINTERTASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "filemanager/filemanager.hpp"

#include "esp_log.h"

void printer_task(void *arg);

extern QueueHandle_t printReqQueue, netAnswQueue, gcodesQueue;
extern FileManager fileManager;

#endif