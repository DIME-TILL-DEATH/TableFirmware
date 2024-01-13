#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "net/wifi.h"
#include "net/tcpip.hpp"

#include "requestactions.h"

#include "projdefines.h"
#include "printer/printsequence.hpp"
#include "printer/printertask.hpp"
#include "filemanager/filemanager.hpp"
#include "netcomm/abstractcommand.hpp"
#include "netcomm/playlistcommand.hpp"

FileManager fileManager;

void sendPlaylistAnswer()
{
    NetComm::PlaylistCommand* answer = new NetComm::PlaylistCommand(0, Requests::Playlist::REQUEST_PLAYLIST, NetComm::ANSWER);
    answer->playlist_ptr = fileManager.getPlaylist_ptr();
    xQueueSendToBack(netAnswQueue, &answer, pdMS_TO_TICKS(10));
}

void sendPlaylistPositionAnswer()
{
    NetComm::PlaylistCommand* answer = new NetComm::PlaylistCommand(0, Requests::Playlist::REQUEST_PLAYLIST_POSITION, NetComm::ANSWER);
    answer->curPlsPos = fileManager.getCurrentPosition();
    xQueueSendToBack(netAnswQueue, &answer, pdMS_TO_TICKS(10));
}

void processNetRequest(NetComm::PlaylistCommand* recvComm)
{
    switch(recvComm->action())
    {
        case Requests::Playlist::REQUEST_PLAYLIST:
        {
            sendPlaylistAnswer();
            break;
        }

        case Requests::Playlist::REQUEST_PLAYLIST_POSITION:
        {
            sendPlaylistPositionAnswer();
            break;
        }

        case Requests::Playlist::CHANGE_PLAYLIST:
        {
            if(recvComm->playlist_ptr)
            {
                fileManager.changePlaylist(recvComm->playlist_ptr);
                delete(recvComm->playlist_ptr);
            }
            sendPlaylistAnswer();        
            break;
        }

        case Requests::Playlist::CHANGE_PLAYLIST_POSITION:
        {
            fileManager.changePlaylistPos(recvComm->curPlsPos);
            sendPlaylistPositionAnswer();
            break;
        }
    }
}

void file_task(void *arg)
{
  FM_RESULT result;
  do
  {
    result = fileManager.connectSDCard();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }while(result != FM_OK);

  fileManager.loadPlaylist();

  for(;;)
  {
      if(uxQueueSpacesAvailable(gcodesQueue))
      {
        GCode::GAbstractComm* nextComm = fileManager.readNextComm();
        if(nextComm)
        {
          xQueueSendToBack(gcodesQueue, &nextComm, portMAX_DELAY);            
        }
        else
        {
            do
            {
                result = fileManager.loadNextPrint();
            }
            while(result != FM_OK);
        } 
      }
      else
      {
        NetComm::PlaylistCommand* recvAction;
        portBASE_TYPE xStatus = xQueueReceive(fileReqQueue, &recvAction, pdMS_TO_TICKS(0));
        if(xStatus == pdPASS)
        {
            processNetRequest(recvAction);
            delete(recvAction);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
      }
  }
}