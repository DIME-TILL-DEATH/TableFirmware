#include "hwtask.hpp"

#include "esp_log.h"
#include "driver/mcpwm_prelude.h"

#include "requestactions.h"
#include "projdefines.h"
#include "printer.hpp"
#include "ledstrip.hpp"
#include "printsequence.hpp"
#include "filemanager/filemanager.hpp"
#include "netcomm/abstractcommand.hpp"
#include "netcomm/hardwarecommand.hpp"

#include "filemanager/settings.hpp"



static const char *TAG = "HW TASK";

Printer printer;
LedStrip* ledStrip;

StatisticData_t statData;

void processNetRequest(NetComm::HardwareCommand* command)
{
    NetComm::HardwareCommand* answer = new NetComm::HardwareCommand(*command);
    answer->direction = NetComm::ANSWER;

    switch(command->action())
    {
        case Requests::Hardware::GET_SERIAL_ID:
        {
            std::string* serialId_ptr = new std::string;
            *serialId_ptr = Settings::getSetting(Settings::String::SERIAL_ID);
            answer->dataPtr = serialId_ptr;
            break;
        }

        case Requests::Hardware::PAUSE_PRINTING:
        {
            delete answer;
            answer = nullptr;

            ESP_LOGI(TAG, "Request pause print");
            printer.pause();
            break;
        }

        case Requests::Hardware::REQUEST_PROGRESS:
        {
            answer->progress.currentPoint = printer.currentPrintPointNum();
            answer->progress.printPoints = fileManager.pointsNum();
            break;
        }

        case Requests::Hardware::SET_PRINT_SPEED:
        {
            float_t newPrintSpeed = command->printSpeed;
            printer.setSpeed(newPrintSpeed);
            answer->printSpeed = newPrintSpeed;
            ESP_LOGI(TAG, "Settled speed: %f", newPrintSpeed);

            Settings::saveSetting(Settings::Digit::PRINT_SPEED, newPrintSpeed);
            break;         
        }

        case Requests::Hardware::GET_PRINT_SPEED:
        {
            answer->printSpeed = printer.getSpeed();
            break;
        }

        case Requests::Hardware::SET_LED_BRIGHTNESS:
        {
            if(ledStrip)
            {
                float_t newBrightness = command->ledBrightness;
                ledStrip->setBrightness(newBrightness);
                answer->ledBrightness= newBrightness;
                ESP_LOGI(TAG, "Settled brightness: %f", newBrightness);
                Settings::saveSetting(Settings::Digit::LED_BRIGHTNESS, newBrightness);           
            }
            break;  
        }

        case Requests::Hardware::GET_LED_BRIGHTNESS:
        {
            if(ledStrip)
            {
                answer->ledBrightness = ledStrip->getBrightness();
            }
            break;
        }

        case Requests::Hardware::SET_SCALE_COEFFICIENT:
        {
            float_t newScaleCoefficient = command->scaleCoefficient;
            answer->scaleCoefficient = newScaleCoefficient;
            ESP_LOGI(TAG, "Settled scale coefficient: %f", newScaleCoefficient);
            Settings::saveSetting(Settings::Digit::SCALE_COEF, newScaleCoefficient);           
            break;  
        }

        case Requests::Hardware::GET_SCALE_COEFFICIENT:
        {
            answer->scaleCoefficient = Settings::getSetting(Settings::Digit::SCALE_COEF);
            break;
        }

        case Requests::Hardware::SET_ROTATION:
        {
            float_t newRotation = command->rotation;
            answer->rotation = newRotation;
            ESP_LOGI(TAG, "Settled rotation: %f", newRotation);
            Settings::saveSetting(Settings::Digit::PRINT_ROTATION, newRotation);           
            break;  
        }

        case Requests::Hardware::GET_ROTATION:
        {
            answer->rotation = Settings::getSetting(Settings::Digit::PRINT_ROTATION);
            break;
        }

        case Requests::Hardware::SET_CORRECTION:
        {
            float_t newCorrection = command->correction;
            answer->scaleCoefficient = newCorrection;
            ESP_LOGI(TAG, "Settled correction: %f", newCorrection);
            Settings::saveSetting(Settings::Digit::CORRETION_LENGTH, newCorrection);           
            break;  
        }

        case Requests::Hardware::GET_CORRECTION:
        {
            answer->correction = Settings::getSetting(Settings::Digit::CORRETION_LENGTH);
            break;
        }

        case Requests::Hardware::SET_PAUSE_INTERVAL:
        {
            uint32_t newPauseInterval = command->pauseInterval;
            ESP_LOGI(TAG, "Settled pause interval: %d", newPauseInterval);
            printer.setPauseInterval(newPauseInterval);
            Settings::saveSetting(Settings::Digit::PAUSE_INTERVAL, newPauseInterval);       
            break;
        }

        case Requests::Hardware::GET_PAUSE_INTERVAL:
        {
            answer->pauseInterval = printer.getPauseInterval();
            break;
        }

        case Requests::Hardware::GET_FI_GEAR2_TEETH_COUNT:
        {
            answer->fiGear2Teeths = printer.getFiGear2TeethsCount();
            break;
        }

        case Requests::Hardware::GET_MACHINE_MINUTES:
        {
            answer->dataPtr = &(statData.machineMinutes);
            answer->dataSize = sizeof(statData.machineMinutes);
            break;
        }
    }

    if(answer)
    {
        xQueueSendToBack(netAnswQueue, &answer, pdMS_TO_TICKS(100));
    }
}

bool firstCommRecv = false;
void hardware_task(void *arg)
{
  xTaskCreatePinnedToCore(statistic_task, "statistic", 4096, NULL, PRIORITY_STATISTIC_TASK, NULL, 1);

  Printer_Init();
  ledStrip = new LedStrip();

  printer.findCenter();
  
  for(;;)
  { 
    portBASE_TYPE xStatus;
    //==========================Main routine============================================
    if(printer.isPrinterFree())
    {
      GCode::GAbstractComm* recvComm;
      xStatus = xQueueReceive(gcodesQueue, &recvComm, pdMS_TO_TICKS(0));
      if(xStatus == pdPASS)
      {
         printer.setNextCommand(recvComm);
         firstCommRecv = true;
      }
      else
      {
         if(firstCommRecv) ESP_LOGE("PRINTER TASK", "queue empty");
      }
    }
    printer.printRoutine();

    //===========================Statistic=============================================
    xStatus = xQueueReceive(statisticQueue, &statData, pdMS_TO_TICKS(0));
    if(xStatus == pdPASS)
    {
        //ESP_LOGI(TAG, "Machine minutes: %d", statData.machineMinutes);  
        Settings::saveSetting(Settings::Digit::MACHINE_MINUTES, statData.machineMinutes);
    }

    //===========================Net request===========================================
    NetComm::HardwareCommand* recvAction;
    xStatus = xQueueReceive(printReqQueue, &recvAction, pdMS_TO_TICKS(0));
    if(xStatus == pdPASS)
    {
        processNetRequest(recvAction);
        delete(recvAction);
    }


    vTaskDelay(pdMS_TO_TICKS(1));
  }
}