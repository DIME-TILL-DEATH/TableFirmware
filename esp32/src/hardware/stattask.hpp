#ifndef STATTASK_H
#define STATTASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "filemanager/filemanager.hpp"

#include "esp_log.h"

typedef struct
{
    uint32_t machineMinutes;
}StatisticData_t;


void statistic_task(void *arg);

#endif