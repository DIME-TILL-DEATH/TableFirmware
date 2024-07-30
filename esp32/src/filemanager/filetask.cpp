#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "ff.h"
#include <dirent.h>
#include <stdio.h>

#include "esp_log.h"
#include "esp_app_desc.h"

#include "net/wifi.hpp"
#include "net/tcpip.hpp"

#include "requestactions.h"

#include "projdefines.h"
#include "hardware/printsequence.hpp"
#include "hardware/hwtask.hpp"
#include "filemanager/filemanager.hpp"

#include "messages/abstractmessage.h"
#include "messages/intvaluemessage.h"
#include "messages/floatvaluemessage.h"
#include "messages/stringmessage.h"
#include "messages/foldercontentmessage.h"
#include "messages/filepartmessage.h"

#include "firmware.hpp"

#include "settings.hpp"

static const char *TAG = "FILE TASK";

AbstractMessage* playlistActions(AbstractMessage* msg)
{
    switch((Requests::Playlist)msg->action())
    {
        case Requests::Playlist::REQUEST_PLAYLIST:
        {
            QString* resultString = new QString; // On heap. Big size
            std::vector<std::string>* playlist_ptr = fileManager.getPlaylist_ptr();
            for(auto it = playlist_ptr->begin(); it != playlist_ptr->end(); ++it)
            {
                resultString->append(*it);
            }

            AbstractMessage* answerMessage = new StringMessage(FrameType::PLAYLIST_ACTIONS, msg->action(), *resultString);
            delete(resultString);
            return answerMessage;
        }

        case Requests::Playlist::REQUEST_PLAYLIST_POSITION:
        {
            return new IntValueMessage(FrameType::PLAYLIST_ACTIONS, msg->action(), fileManager.getCurrentPosition());
        }

        case Requests::Playlist::CHANGE_PLAYLIST:
        {
            StringMessage* stringMsg = static_cast<StringMessage*>(msg);
            fileManager.changePlaylist(stringMsg->string());

            //answer
            QString* resultString = new QString; // On heap. Big size
            std::vector<std::string>* playlist_ptr = fileManager.getPlaylist_ptr();
            for(auto it = playlist_ptr->begin(); it != playlist_ptr->end(); ++it)
            {
                resultString->append(*it);
            }
            AbstractMessage* answerMessage = new StringMessage(FrameType::PLAYLIST_ACTIONS, msg->action(), *resultString);
            delete(resultString);      
            return answerMessage;
        }

        case Requests::Playlist::CHANGE_PLAYLIST_POSITION:
        {
            IntValueMessage* intMsg = static_cast<IntValueMessage*>(msg);

            ESP_LOGI("FILE TASK", "Change playlist position");
            fileManager.changePlaylistPos(intMsg->value());
            return new IntValueMessage(FrameType::PLAYLIST_ACTIONS, msg->action(), fileManager.getCurrentPosition());

        }

        case Requests::Playlist::CHANGE_PRINTNG_FILE:
        {
            IntValueMessage* intMsg = static_cast<IntValueMessage*>(msg);
            int32_t newFilePosition = intMsg->value();

            printer.abortPoint();

            ESP_LOGI("FILE TASK", "Request to change printng file, pos: %d", newFilePosition);
            xQueueReset(gcodesQueue);
            printer.stop();
            fileManager.loadPrintFromPlaylist(newFilePosition);

            return new IntValueMessage(FrameType::PLAYLIST_ACTIONS, msg->action(), fileManager.getCurrentPosition());
        }

        case Requests::Playlist::GET_CURRENT_GALLERY:
        {
            return new StringMessage(FrameType::PLAYLIST_ACTIONS, (uint8_t)Requests::Playlist::GET_CURRENT_GALLERY, fileManager.currentGalleryName());
        }

        case Requests::Playlist::SET_CURRENT_GALLERY:
        {
            StringMessage* stringMsg = static_cast<StringMessage*>(msg);
            QString galleryName = stringMsg->string();

            Settings::saveSetting(Settings::String::PRINT_GALLERY, galleryName);

            ESP_LOGI("FILE TASK", "Request to set new gallery: %s", galleryName.c_str());
            fileManager.loadGallery(galleryName, 0);
            xQueueReset(gcodesQueue);
            printer.stop();
            fileManager.loadNextPrint();

            return new StringMessage(FrameType::PLAYLIST_ACTIONS, (uint8_t)Requests::Playlist::GET_CURRENT_GALLERY, fileManager.currentGalleryName());
        }
    }
    return nullptr;
}

AbstractMessage* fileActions(AbstractMessage* msg)
{
    switch((Requests::File)msg->action())
    {
        case Requests::File::GET_FOLDER_CONTENT:
        {            
            StringMessage* stringMsg = static_cast<StringMessage*>(msg);
            QString dirPath = stringMsg->string();

            DIR* dir = opendir(dirPath.c_str());
            if(dir == NULL)
            {
                ESP_LOGE(TAG, "opendir() failed, path: %s", dirPath.c_str());
                return nullptr;
            }
            ESP_LOGI(TAG, "Dir  %s opened, sending answer", dirPath.c_str());

            dirent *entry;
            QStringList result;
            while ((entry = readdir(dir)) != NULL) 
            {
                if(entry->d_type == DT_REG)
                {
                    result.push_back(QString(entry->d_name));
                }
                else if(entry->d_type == DT_DIR)
                {
                    char rootName[128] = "System volume information";
                    if(strcmp(entry->d_name, rootName))
                    {
                        result.push_back(std::string("DIR|") + QString(entry->d_name));
                    }
                }
            }
            closedir(dir);
            
            return new FolderContentMessage(dirPath, result);
        }

        case Requests::File::GET_FILE:
        {
            StringMessage* stringMsg = static_cast<StringMessage*>(msg);
            fileManager.appendFileRequest(stringMsg->string());
            ESP_LOGI("FILE TASK", "Requesting file: %s", stringMsg->string().c_str());
            break;
        }

        case Requests::File::FILE_CREATE:
        {
            FilePartMessage* fileMsg = static_cast<FilePartMessage*>(msg);

            FileManager::fileWrite(fileMsg->dstPath().c_str(), "w", fileMsg->filePart().data(), fileMsg->filePart().size());
            return new FilePartMessage(FilePartMessage::ActionType::CREATE_TEXT, fileMsg->dstPath(), fileMsg->dstPath(), 
                                        QByteArray(), fileMsg->partPosition() + fileMsg->filePart().size());
        }

        case Requests::File::FILE_APPEND_DATA:
        {
            FilePartMessage* fileMsg = static_cast<FilePartMessage*>(msg);

            FileManager::fileWrite(fileMsg->dstPath().c_str(), "a", fileMsg->filePart().data(), fileMsg->filePart().size());
            return new FilePartMessage(FilePartMessage::ActionType::APPEND_TEXT, fileMsg->dstPath(), fileMsg->dstPath(), 
                                        QByteArray(), fileMsg->partPosition() + fileMsg->filePart().size());
        }
    }
    return nullptr;
}

AbstractMessage* firmwareActions(AbstractMessage* msg)
{
    switch((Requests::Firmware)msg->action())
    {
        case Requests::Firmware::FIRMWARE_VERSION:
        {
            std::string versionString = esp_app_get_description()->version;
            return new StringMessage(FrameType::FIRMWARE_ACTIONS, msg->action(), versionString);
        }

        case Requests::Firmware::FIRMWARE_UPLOAD_START:
        {
            FilePartMessage* fileMsg = static_cast<FilePartMessage*>(msg);

            FileManager::fileWrite(fileMsg->dstPath().c_str(), "wb", fileMsg->filePart().data(), fileMsg->filePart().size());
            return new FilePartMessage(FilePartMessage::ActionType::CREATE_FIRMWARE, fileMsg->dstPath(), fileMsg->dstPath(), 
                                        QByteArray(), fileMsg->partPosition() + fileMsg->filePart().size());
        }

        case Requests::Firmware::FIRMWARE_UPLOAD_PROCEED:
        {
            FilePartMessage* fileMsg = static_cast<FilePartMessage*>(msg);
            FileManager::fileWrite(fileMsg->dstPath().c_str(), "ab", fileMsg->filePart().data(), fileMsg->filePart().size());
            return new FilePartMessage(FilePartMessage::ActionType::APPEND_FIRMWARE, fileMsg->dstPath(), fileMsg->dstPath(), 
                                        QByteArray(), fileMsg->partPosition() + fileMsg->filePart().size());
        }

        case Requests::Firmware::FIRMWARE_UPDATE:
        {
            printer.pauseThread();
            uint32_t resultUpdate = FW_DoFirmwareUpdate();
            ESP_LOGI("UPDATE", "Send frame FIRMWARE_UPDATE");
            return new IntValueMessage(FrameType::FIRMWARE_ACTIONS, (uint8_t)Requests::Firmware::FIRMWARE_UPDATE, resultUpdate);
        }

        case Requests::Firmware::ESP_RESTART:
        {
            FW_RestartEsp();
            break;
        }
    }
    return nullptr;
}

void processFileMessage(AbstractMessage* msg)
{
    AbstractMessage* answerMessage = nullptr;

    switch(msg->frameType())
    {
        case FrameType::HARDWARE_ACTIONS: ESP_LOGE(TAG, "HARDWARE_ACTION message in fileTask"); break;

        case FrameType::PLAYLIST_ACTIONS:
        {
            answerMessage = playlistActions(msg);
            break;
        }

        case FrameType::FILE_ACTIONS:
        {
            answerMessage = fileActions(msg);
            break;
        }

        case FrameType::FIRMWARE_ACTIONS:
        {
            answerMessage = firmwareActions(msg);
            break;
        }

        default: break;
    }

    if(answerMessage)
    {
        xQueueSendToBack(netAnswQueue, &answerMessage, portMAX_DELAY);//pdMS_TO_TICKS(100));
    }
}

void file_task(void *arg)
{
  FM_RESULT result;

  uint32_t playlstPosition = Settings::getSetting(Settings::Digit::LAST_PLAYLIST_POSITION);
  std::string galleryName = Settings::getSetting(Settings::String::PRINT_GALLERY);
  
  if(galleryName != "")
  {
    fileManager.loadGallery(galleryName, playlstPosition);
    ESP_LOGI("FM TASK", "Loading gallery %s in position %d", galleryName.c_str(), fileManager.getCurrentPosition()+1);
  }
  else
  {
    fileManager.loadPlaylist(playlstPosition);
    ESP_LOGI("FM TASK", "Loading default playlist in position %d", fileManager.getCurrentPosition()+1);
  }

  FilePartMessage* filePartMsg = nullptr;
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
                ESP_LOGI("FM TASK", "loading next print");
                result = fileManager.loadNextPrint();
                if(result == FM_OK)
                {
                    ESP_LOGI("FM TASK", "loaded");
                }
                else
                {
                    ESP_LOGE("FM TASK", "Load next print failed.");
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    break;
                }
            }
            while(result != FM_OK);
        } 
    }

    AbstractMessage* recvMsg;
    portBASE_TYPE xStatus = xQueueReceive(fileReqQueue, &recvMsg, pdMS_TO_TICKS(0));
    if(xStatus == pdPASS)
    {
        processFileMessage(recvMsg);
        delete(recvMsg);
    }

    
    if(filePartMsg)
    {
        xStatus = xQueueSendToBack(netAnswQueue, &filePartMsg, pdMS_TO_TICKS(25));
        
        if(xStatus == pdPASS)
        {
            filePartMsg = nullptr;
        }
    }
    else
    {
        filePartMsg = fileManager.getRequestedData();
    }
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}