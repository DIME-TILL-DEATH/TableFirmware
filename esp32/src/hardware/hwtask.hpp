#ifndef PRINTERTASK_H
#define PRINTERTASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "filemanager/filemanager.hpp"

#include "hardware/stattask.hpp"

#include "esp_log.h"

void hardware_task(void *arg);

extern QueueHandle_t printReqQueue, netAnswQueue, gcodesQueue, statisticQueue;
extern FileManager fileManager;

#endif