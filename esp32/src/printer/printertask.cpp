#include "printertask.hpp"

#include "requestactions.h"
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
        case Requests::Transport::PAUSE_PRINTING:
        {
            
            break;
        }

        case Requests::Transport::REQUEST_PROGRESS:
        {
            NetComm::TransportCommand* answer = new NetComm::TransportCommand(0, Requests::Transport::REQUEST_PROGRESS, NetComm::ANSWER);
            answer->progress.currentPoint = printer.currentPrintPointNum();
            answer->progress.printPoints = fileManager.pointsNum();
            xQueueSendToBack(netAnswQueue, &answer, pdMS_TO_TICKS(10));
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