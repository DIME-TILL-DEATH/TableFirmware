#include "hwtask.hpp"

#include "esp_log.h"
#include "driver/mcpwm_prelude.h"

#include "requestactions.h"
#include "projdefines.h"
#include "printer.hpp"
#include "ledstrip.hpp"
#include "printsequence.hpp"

#include "filemanager/filemanager.hpp"
#include "filemanager/settings.hpp"

#include "utils/qt_compat.h"

#include "messages/abstractmessage.h"
#include "messages/intvaluemessage.h"
#include "messages/floatvaluemessage.h"
#include "messages/stringmessage.h"


static const char *TAG = "HW TASK";

Printer printer;
LedStrip* ledStrip;

StatisticData_t statData;

void processMessage(AbstractMessage* msg)
{
    if(msg->frameType() != FrameType::HARDWARE_ACTIONS)
    {
        ESP_LOGE(TAG, "Not HARDWARE_ACTION message in hwTask");
        return;
    }

    AbstractMessage* answerMessage = nullptr;

    switch(static_cast<Requests::Hardware>(msg->action()))
    {
        case Requests::Hardware::GET_SERIAL_ID:
        {
            answerMessage = new StringMessage(FrameType::HARDWARE_ACTIONS, msg->action(),  Settings::getSetting(Settings::String::SERIAL_ID));
            break;
        }

        case Requests::Hardware::PAUSE_PRINTING:
        {
            ESP_LOGI(TAG, "Request pause print");
            printer.pauseResume();
            break;
        }

        case Requests::Hardware::REQUEST_PROGRESS:
        {
            answerMessage = new IntValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), printer.currentPrintPointNum(), fileManager.pointsNum());
            break;
        }

        case Requests::Hardware::SET_PRINT_SPEED:
        {
            FloatValueMessage* floatMsg = static_cast<FloatValueMessage*>(msg);
            float_t newPrintSpeed = floatMsg->value();
            printer.setSpeed(newPrintSpeed);

            ESP_LOGI(TAG, "Settled speed: %f", newPrintSpeed);
            Settings::saveSetting(Settings::Digit::PRINT_SPEED, newPrintSpeed);
            answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), newPrintSpeed);
            break;         
        }

        case Requests::Hardware::GET_PRINT_SPEED:
        {
            answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), printer.getSpeed());
            break;
        }

        case Requests::Hardware::SET_LED_BRIGHTNESS:
        {
            if(ledStrip)
            {
                FloatValueMessage* floatMsg = static_cast<FloatValueMessage*>(msg);
                float_t newBrightness = floatMsg->value();;
                ledStrip->setBrightness(newBrightness);

                ESP_LOGI(TAG, "Settled brightness: %f", newBrightness);
                Settings::saveSetting(Settings::Digit::LED_BRIGHTNESS, newBrightness);  
                answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), newBrightness);         
            }
            break;  
        }

        case Requests::Hardware::GET_LED_BRIGHTNESS:
        {
            if(ledStrip)
            {
                answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), ledStrip->getBrightness());
            }
            break;
        }

        case Requests::Hardware::SET_SCALE_COEFFICIENT:
        {
            FloatValueMessage* floatMsg = static_cast<FloatValueMessage*>(msg);
            float_t newScaleCoefficient = floatMsg->value();

            ESP_LOGI(TAG, "Settled scale coefficient: %f", newScaleCoefficient);
            Settings::saveSetting(Settings::Digit::SCALE_COEF, newScaleCoefficient);
            answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), newScaleCoefficient);           
            break;  
        }

        case Requests::Hardware::GET_SCALE_COEFFICIENT:
        {
            answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), Settings::getSetting(Settings::Digit::SCALE_COEF));
            break;
        }

        case Requests::Hardware::SET_ROTATION:
        {
            FloatValueMessage* floatMsg = static_cast<FloatValueMessage*>(msg);
            float_t newRotation = floatMsg->value();

            ESP_LOGI(TAG, "Settled rotation: %f", newRotation);
            Settings::saveSetting(Settings::Digit::PRINT_ROTATION, newRotation);  
            answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), newRotation);         
            break;  
        }

        case Requests::Hardware::GET_ROTATION:
        {
            answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), Settings::getSetting(Settings::Digit::PRINT_ROTATION));
            break;
        }

        case Requests::Hardware::SET_CORRECTION:
        {
            FloatValueMessage* floatMsg = static_cast<FloatValueMessage*>(msg);
            float_t newCorrection = floatMsg->value();;

            ESP_LOGI(TAG, "Settled correction: %f", newCorrection);
            Settings::saveSetting(Settings::Digit::CORRETION_LENGTH, newCorrection);  
            answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), newCorrection);         
            break;  
        }

        case Requests::Hardware::GET_CORRECTION:
        {
            answerMessage = new FloatValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), Settings::getSetting(Settings::Digit::CORRETION_LENGTH));
            break;
        }

        case Requests::Hardware::SET_PAUSE_INTERVAL:
        {
            IntValueMessage* intMsg = static_cast<IntValueMessage*>(msg);
            uint32_t newPauseInterval = intMsg->value();

            ESP_LOGI(TAG, "Settled pause interval: %d", newPauseInterval);
            printer.setPauseInterval(newPauseInterval);
            Settings::saveSetting(Settings::Digit::PAUSE_INTERVAL, newPauseInterval);      
            answerMessage = new IntValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), printer.getPauseInterval()); 
            break;
        }

        case Requests::Hardware::GET_PAUSE_INTERVAL:
        {
            answerMessage = new IntValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), printer.getPauseInterval());
            break;
        }

        case Requests::Hardware::GET_FI_GEAR2_TEETH_COUNT:
        {
            answerMessage = new IntValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), printer.getFiGear2TeethsCount());
            break;
        }

        case Requests::Hardware::GET_MACHINE_MINUTES:
        {
            answerMessage = new IntValueMessage(FrameType::HARDWARE_ACTIONS, msg->action(), statData.machineMinutes);
            break;
        }

        default: ESP_LOGE(TAG, "Unknown hardware action"); break;
    }

    if(answerMessage)
    {
        xQueueSendToBack(netAnswQueue, &answerMessage, pdMS_TO_TICKS(100));
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
         //if(firstCommRecv) ESP_LOGE("PRINTER TASK", "queue empty");
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
    AbstractMessage* recvMsg;
    xStatus = xQueueReceive(printReqQueue, &recvMsg, pdMS_TO_TICKS(0));
    if(xStatus == pdPASS)
    {
        processMessage(recvMsg);
        delete(recvMsg);
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}