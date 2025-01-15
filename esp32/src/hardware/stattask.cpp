#include "stattask.hpp"

#include "filemanager/settings.hpp"

static const char *TAG = "STATISTIC";

QueueHandle_t statisticQueue;

StatisticData_t gatheredStatistic;

void statistic_task(void *arg)
{
    statisticQueue = xQueueCreate(4, sizeof(StatisticData_t));

    gatheredStatistic.machineMinutes = Settings::getSetting(Settings::Digit::MACHINE_MINUTES);
    for(;;)
    {   
        char buffer[1024]; 
        xQueueSendToBack(statisticQueue, &gatheredStatistic, pdMS_TO_TICKS(10));
        // vTaskGetRunTimeStats(buffer);
        // ESP_LOGI(TAG, "%s", buffer);
        vTaskDelay(pdMS_TO_TICKS(60000));
        gatheredStatistic.machineMinutes++;         
    }
}