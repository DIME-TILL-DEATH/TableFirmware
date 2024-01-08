#include "printertask.hpp"

#include "projdefines.h"
#include "printer.hpp"
#include "printsequence.hpp"
#include "filemanager/filemanager.hpp"
#include "netcomm/abstractcommand.hpp"
#include "netcomm/transportcommand.hpp"

Printer printer;

void processNetRequest(NetComm::TransportCommand* command)
{
    switch(command->action())
    {
        case NetComm::TransportCommand::PREVIOUS_PRINT:
        {
            break;
        }

        case NetComm::TransportCommand::PAUSE_PRINTING:
        {
            
            break;
        }

        case NetComm::TransportCommand::NEXT_PRINT:
        {
            break;
        }

        case NetComm::TransportCommand::REQUEST_PROGRESS:
        {
            NetComm::TransportCommand* answer = new NetComm::TransportCommand(0, NetComm::TransportCommand::REQUEST_PROGRESS, NetComm::ANSWER);
            answer->progress.currentPoint = printer.currentPrintPointNum();
            answer->progress.printPoints = fileManager.pointsNum();
            xQueueSendToBack(netAnswQueue, &answer, pdMS_TO_TICKS(10));
            break;
        }

        case NetComm::TransportCommand::SET_PRINT:
        {
            break;
        }
    }
}

void printer_task(void *arg)
{
  for(;;)
  { 
    if(printer.isPrinterFree())
    {
      GCode::GAbstractComm* recvComm;
      portBASE_TYPE xStatus = xQueueReceive(gcodesQueue, &recvComm, pdMS_TO_TICKS(0));
      if(xStatus == pdPASS)
      {
         printer.setNextCommand(recvComm);
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