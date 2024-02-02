#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "net/wifi.hpp"
#include "net/tcpip.hpp"

#include "requestactions.h"

#include "projdefines.h"
#include "printer/printsequence.hpp"
#include "printer/printertask.hpp"
#include "filemanager/filemanager.hpp"
#include "netcomm/abstractcommand.hpp"
#include "netcomm/playlistcommand.hpp"

#include "settings.hpp"

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

        case Requests::Playlist::CHANGE_PRINTNG_FILE:
        {
            ESP_LOGI("FILE TASK", "Request to change printng file");
            xQueueReset(gcodesQueue);
            fileManager.loadPrintFromPlaylist(recvComm->curPlsPos);
            sendPlaylistPositionAnswer();
            break;
        }
    }
}

void file_task(void *arg)
{
  FM_RESULT result;

  std::string playlistName = Settings::getStringSetting(Settings::String::PLAYLIST);
  uint32_t playlstPosition = Settings::getDigitSetting(Settings::Digit::LAST_PLAYLIST_POSITION);

  fileManager.loadPlaylist(playlistName, playlstPosition);
  ESP_LOGI("FM TASK", "Loading playlist %s in position %d", playlistName.c_str(), playlstPosition);

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