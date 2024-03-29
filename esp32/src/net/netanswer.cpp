#include "netanswer.hpp"

#include <string.h>
#include <vector>
#include "esp_log.h"

#include "ff.h"
#include <dirent.h>
#include <stdio.h>

#include "filemanager/filemanager.hpp"
#include "frames.h"
#include "requestactions.h"
#include "firmware.hpp"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

static const char *TAG = "NET ANSWER";
#define TX_BUFFER_SIZE 1024

void sendData(int socket, void* txBuffer, size_t len)
{
    if(len>0)
    {
        int written = send(socket, txBuffer, len, 0);
        if (written < 0) 
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        }
    }
}

void sendLongVector(int socket, FrameHeader_uni& answerFrameHeader, std::vector<std::string>* data)
{
    char txBuffer[TX_BUFFER_SIZE];

    uint16_t len = sizeof(FrameHeader);

    for(auto it = data->begin(); it!=data->end(); ++it)
    {
        len += (*it).size();
    }

    answerFrameHeader.structData.frameSize = len;
    
    int parts = len / TX_BUFFER_SIZE + 1;
    
    memcpy(txBuffer, answerFrameHeader.rawData, sizeof(FrameHeader));
    auto curItem = data->begin();
    int curSendBytes = sizeof(FrameHeader);

    for(int i=0; i<parts; i++)
    {
        while((curItem != data->end()) && ((curSendBytes + (*curItem).size()) < TX_BUFFER_SIZE))
        {
            memcpy(&txBuffer[curSendBytes], (*curItem).data(), (*curItem).size());
            curSendBytes += (*curItem).size();
            curItem++;
        }
        sendData(socket, txBuffer, curSendBytes);
        curSendBytes = 0;
    }
}

void processHardwareCommand(NetComm::HardwareCommand* hardwareAnswer, int socket)
{
    uint8_t buffer[512];
    uint32_t dataSize;

    hardwareAnswer->formAnswerFrame(buffer, &dataSize);

    sendData(socket, buffer, dataSize);
}

void processPlaylistCommand(NetComm::PlaylistCommand* playlistAnswer, int socket)
{   
    FrameHeader_uni answerFrameHeader;
    memset(answerFrameHeader.rawData, 0, sizeof(FrameHeader));
    answerFrameHeader.structData.frameType = FrameType::PLAYLIST_ACTIONS;

    switch(playlistAnswer->action())
    {
        case Requests::Playlist::CHANGE_PLAYLIST:
        {
            //break;
        }

        case Requests::Playlist::REQUEST_PLAYLIST:
        {
            std::vector<std::string>* playlist = playlistAnswer->playlist_ptr;

            if(!playlist)
            {
                ESP_LOGE(TAG, "Playlis answer, pointer to playlis unavaliable");
                return;
            }
            answerFrameHeader.structData.action = (uint8_t)Requests::Playlist::REQUEST_PLAYLIST;

            printf("Playlist: ");
            for(auto it=playlist->begin(); it!=playlist->end(); it++)
            {
                printf("%s  ", (*it).c_str());
            }
            printf("\r\n");

            sendLongVector(socket, answerFrameHeader, playlist);
            break;
        }

        case Requests::Playlist::CHANGE_PLAYLIST_POSITION:
        {
            //break;
        }

        case Requests::Playlist::CHANGE_PRINTNG_FILE:
        {
           // break;
        }

        case Requests::Playlist::REQUEST_PLAYLIST_POSITION:
        {
            //ESP_LOGI("Answer", "ready to answer, playlist position:%d", playlistAnswer->curPlsPos);
            answerFrameHeader.structData.action = (uint8_t)Requests::Playlist::REQUEST_PLAYLIST_POSITION;
            answerFrameHeader.structData.frameSize = sizeof(FrameHeader);
            answerFrameHeader.structData.data0 = playlistAnswer->curPlsPos;

            sendData(socket, answerFrameHeader.rawData, sizeof(FrameHeader));
            break;
        }
    }
}

void processFileCommand(NetComm::FileCommand* fileAnswer, int socket)
{
    FrameHeader_uni answerFrameHeader;
    memset(answerFrameHeader.rawData, 0, sizeof(FrameHeader));
    answerFrameHeader.structData.frameType = FrameType::FILE_ACTIONS;

    switch(fileAnswer->action())
    {
        case Requests::File::GET_FILE:
        {
            std::string fileName = fileAnswer->path;
            FILE* reqFile = fopen(fileName.c_str(), "r");
            if(reqFile)
            {
                fseek(reqFile, 0 , SEEK_END);
                long fileSize = ftell(reqFile);
                ESP_LOGI(TAG, "File  %s opened. Size: %d, sending answer", fileName.c_str(), fileSize);
                fseek(reqFile, 0 , SEEK_SET); 

                answerFrameHeader.structData.action = (uint8_t)Requests::File::GET_FILE;
                answerFrameHeader.structData.frameSize = sizeof(FrameHeader) + fileName.size() + fileSize;
                answerFrameHeader.structData.frameParameters = fileName.size();
                answerFrameHeader.structData.data0 = fileSize;

                char buffer[2048];
                memcpy(buffer, answerFrameHeader.rawData, sizeof(FrameHeader));
                memcpy(&buffer[sizeof(FrameHeader)], fileName.c_str(), fileName.size());
                sendData(socket, buffer, sizeof(FrameHeader)+fileName.size());
             
                size_t bytesReaded;
                do
                {
                    bytesReaded = fread(buffer, 1, 2048, reqFile);
                    sendData(socket, buffer, bytesReaded);
                }while(bytesReaded>0);

                fclose(reqFile);
            }
            else
            {
                ESP_LOGE(TAG, "File request. Can't open file %s", fileName.c_str());
                answerFrameHeader.structData.action = (uint8_t)Requests::File::GET_FILE;
                answerFrameHeader.structData.frameSize = sizeof(FrameHeader) + fileName.size();
                answerFrameHeader.structData.frameParameters = fileName.size();
                answerFrameHeader.structData.data0 = -1;

                char buffer[512];
                memcpy(buffer, answerFrameHeader.rawData, sizeof(FrameHeader));
                memcpy(&buffer[sizeof(FrameHeader)], fileName.c_str(), fileName.size());

                sendData(socket, buffer, sizeof(FrameHeader) + fileName.size());
                return;
            }

            break;
        }

        case Requests::File::GET_FOLDER_CONTENT:
        {
            std::string dirPath = fileAnswer->path;
            DIR* dir = opendir(dirPath.c_str());
            if(dir == NULL)
            {
                ESP_LOGE(TAG, "opendir() failed, path: %s", dirPath.c_str());
                return;
            }
            ESP_LOGI(TAG, "Dir  %s opened, sending answer", dirPath.c_str());

            dirent *entry;

            uint32_t totalFiles = 0;
            uint32_t totalDirs = 0;
            std::vector<std::string> result;
            //result.push_back(dirPath + std::string("*"));
            result.push_back(dirPath);
            answerFrameHeader.structData.frameParameters = dirPath.size();

            while ((entry = readdir(dir)) != NULL) 
            {
                if(entry->d_type == DT_REG)
                {
                    result.push_back(entry->d_name + std::string("\r"));
                    totalFiles++;
                }
                else if(entry->d_type == DT_DIR)
                {
                    char rootName[128] = "System volume information";
                    if(strcmp(entry->d_name, rootName))
                    {
                        result.push_back(std::string("DIR|") + entry->d_name + std::string("\r"));
                        totalDirs++;
                    }
                }
            }
            answerFrameHeader.structData.data0 = totalDirs;
            answerFrameHeader.structData.data1 = totalFiles;
            answerFrameHeader.structData.action = (uint8_t)Requests::File::GET_FOLDER_CONTENT;

            sendLongVector(socket, answerFrameHeader, &result);

            closedir(dir);
            break;
        }

        case Requests::File::FILE_CREATE:
        {
            // same as FILE_APPEND_DATA:
        }

        case Requests::File::FILE_APPEND_DATA:
        {
            std::string fileName = fileAnswer->path;
            answerFrameHeader.structData.action = (uint8_t)Requests::File::FILE_APPEND_DATA;
            answerFrameHeader.structData.frameSize = sizeof(FrameHeader) + fileName.size();
            answerFrameHeader.structData.frameParameters = fileName.size();
            answerFrameHeader.structData.data0 = fileAnswer->dataProcessed;

            char buffer[2048];
            memcpy(buffer, answerFrameHeader.rawData, sizeof(FrameHeader));
            memcpy(&buffer[sizeof(FrameHeader)], fileName.c_str(), fileName.size());
            sendData(socket, buffer, sizeof(FrameHeader)+fileName.size());
            break;
        }
    }
}

void processFirmwareCommand(NetComm::FirmwareCommand* firmwareAnswer, int socket)
{
    FrameHeader_uni answerFrameHeader;
    memset(answerFrameHeader.rawData, 0, sizeof(FrameHeader));
    answerFrameHeader.structData.frameType = FrameType::FIRMWARE_ACTIONS;

    switch(firmwareAnswer->action())
    {
    case Requests::Firmware::FIRMWARE_VERSION:
    {
        size_t versionStringSize = firmwareAnswer->firmwareVersion.size();
        answerFrameHeader.structData.action = (uint8_t)Requests::Firmware::FIRMWARE_VERSION;
        answerFrameHeader.structData.frameSize = sizeof(FrameHeader) + versionStringSize;
        answerFrameHeader.structData.frameParameters = versionStringSize;

        char buffer[512];
        memcpy(buffer, answerFrameHeader.rawData, sizeof(FrameHeader));
        memcpy(&buffer[sizeof(FrameHeader)], firmwareAnswer->firmwareVersion.c_str(), versionStringSize);
        sendData(socket, buffer, sizeof(FrameHeader)+versionStringSize);
        
        break;
    }
    case Requests::Firmware::FIRMWARE_UPLOAD_START:
    {   
        // Same as proceed
    }

    case Requests::Firmware::FIRMWARE_UPLOAD_PROCEED:
    {
        answerFrameHeader.structData.action = (uint8_t)Requests::Firmware::FIRMWARE_UPLOAD_PROCEED;
        answerFrameHeader.structData.frameSize = sizeof(FrameHeader);
        answerFrameHeader.structData.data0 = firmwareAnswer->dataProcessed;
        answerFrameHeader.structData.data1 = firmwareAnswer->fileSize;

        char buffer[2048];
        memcpy(buffer, answerFrameHeader.rawData, sizeof(FrameHeader));
        sendData(socket, buffer, sizeof(FrameHeader));
        break;
    }

    case Requests::Firmware::FIRMWARE_UPDATE:
    {
        int resultUpdate = FW_DoFirmwareUpdate();

        answerFrameHeader.structData.action = (uint8_t)Requests::Firmware::FIRMWARE_UPDATE;
        answerFrameHeader.structData.frameSize = sizeof(FrameHeader);
        answerFrameHeader.structData.data0 = resultUpdate;

        ESP_LOGI("UPDATE", "Send frame FIRMWARE_UPDATE");

        char buffer[128];
        memcpy(buffer, answerFrameHeader.rawData, sizeof(FrameHeader));
        sendData(socket, buffer, sizeof(FrameHeader));
        
        break;
    }

    case Requests::Firmware::ESP_RESTART:
    {
        // do nothing
        break;
    }
    }
}

void processAnswer(NetComm::AbstractCommand* recvComm, int socket)
{
    switch (recvComm->commandType())
    {
        case NetComm::ABSTRACT:{break;}
        case NetComm::HARDWARE_COMMAND:
        { 
            NetComm::HardwareCommand* hardwareAnswer = static_cast<NetComm::HardwareCommand*>(recvComm);
            processHardwareCommand(hardwareAnswer, socket);
            break;
        }
        case NetComm::PLAYLIST_COMMAND:
        {
            NetComm::PlaylistCommand* playlistAnswer = static_cast<NetComm::PlaylistCommand*>(recvComm);
            processPlaylistCommand(playlistAnswer, socket);          
            break;
        }
        case NetComm::FILE_COMMAND:
        {
            NetComm::FileCommand* fileAnswer = static_cast<NetComm::FileCommand*>(recvComm);
            processFileCommand(fileAnswer, socket); 
            break;
        }
        case NetComm::FIRMWARE_COMMAND:
        {
            NetComm::FirmwareCommand* firmwareAnswer = static_cast<NetComm::FirmwareCommand*>(recvComm);
            processFirmwareCommand(firmwareAnswer, socket);
            break;
        }
    }
}