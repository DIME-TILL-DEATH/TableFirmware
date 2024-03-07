#include "hwtask.hpp"

#include "requestactions.h"
#include "projdefines.h"
#include "printer.hpp"
#include "printsequence.hpp"
#include "filemanager/filemanager.hpp"
#include "netcomm/abstractcommand.hpp"
#include "netcomm/transportcommand.hpp"

#include "filemanager/settings.hpp"

Printer printer;

void LED_Init()
{

}

void processNetRequest(NetComm::TransportCommand* command)
{
    switch(command->action())
    {
        case Requests::Transport::PAUSE_PRINTING:
        {
            ESP_LOGI("PRINTER TASK", "Request pause print");
            printer.pause();
            break;
        }

        case Requests::Transport::REQUEST_PROGRESS:
        {
            NetComm::TransportCommand* answer = new NetComm::TransportCommand(0, Requests::Transport::REQUEST_PROGRESS, NetComm::ANSWER);
            answer->progress.currentPoint = printer.currentPrintPointNum();
            answer->progress.printPoints = fileManager.pointsNum();
            xQueueSendToBack(netAnswQueue, &answer, pdMS_TO_TICKS(100));
            break;
        }

        case Requests::Transport::SET_PRINT_SPEED:
        {
            NetComm::TransportCommand* answer = new NetComm::TransportCommand(0, Requests::Transport::SET_PRINT_SPEED, NetComm::ANSWER);
            float_t newPrintSpeed = command->printSpeed;
            printer.setSpeed(newPrintSpeed);
            answer->printSpeed = newPrintSpeed;
            ESP_LOGI("PRINTER TASK", "Settled speed: %f", newPrintSpeed);
            xQueueSendToBack(netAnswQueue, &answer, pdMS_TO_TICKS(100));

            Settings::saveSetting(Settings::Digit::PRINT_SPEED, newPrintSpeed);
            break;         
        }

        case Requests::Transport::GET_PRINT_SPEED:
        {
            NetComm::TransportCommand* answer = new NetComm::TransportCommand(0, Requests::Transport::GET_PRINT_SPEED, NetComm::ANSWER);
            answer->printSpeed = printer.getSpeed();
            xQueueSendToBack(netAnswQueue, &answer, pdMS_TO_TICKS(100));
            break;
        }
    }
}

bool firstCommRecv = false;
void hardware_task(void *arg)
{
  Printer_Init();
  LED_Init();

  printer.findCenter();
  
  for(;;)
  { 
    if(printer.isPrinterFree())
    {
      GCode::GAbstractComm* recvComm;
      portBASE_TYPE xStatus = xQueueReceive(gcodesQueue, &recvComm, pdMS_TO_TICKS(0));
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

    NetComm::TransportCommand* recvAction;
    portBASE_TYPE xStatus = xQueueReceive(printReqQueue, &recvAction, pdMS_TO_TICKS(0));
    if(xStatus == pdPASS)
    {
        processNetRequest(recvAction);
        delete(recvAction);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}