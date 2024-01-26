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
        //ESP_LOGI("Answer", "Send part: %d, len: %d", i, curSendBytes);
        sendData(socket, txBuffer, curSendBytes);
        curSendBytes = 0;
    }
}

void processTransportCommand(NetComm::TransportCommand* transportAnswer, int socket)
{
    FrameHeader_uni answerFrame;
    memset(answerFrame.rawData, 0, sizeof(FrameHeader));
    answerFrame.structData.frameType = FrameType::TRANSPORT_ACTIONS;

    switch(transportAnswer->action())
    {
        case Requests::Transport::PAUSE_PRINTING:
        {
            
            break;
        }

        case Requests::Transport::REQUEST_PROGRESS:
        {
           // ESP_LOGI("Answer", "ready to answer, Cur point: %d, All points: %d", transportAnswer->progress.currentPoint, transportAnswer->progress.printPoints);
            
            answerFrame.structData.actionType = (uint8_t)Requests::Transport::REQUEST_PROGRESS;
            answerFrame.structData.frameSize = sizeof(FrameHeader);
            answerFrame.structData.data0 = transportAnswer->progress.currentPoint;    
            answerFrame.structData.data1 = transportAnswer->progress.printPoints; 

            sendData(socket, answerFrame.rawData, sizeof(FrameHeader));
            break;
        }

        case Requests::Transport::SET_PRINT:
        {
            break;
        }
    }
}

void processPlaylistCommand(NetComm::PlaylistCommand* playlistAnswer, int socket)
{   
    FrameHeader_uni answerFrameHeader;
    memset(answerFrameHeader.rawData, 0, sizeof(FrameHeader));
    answerFrameHeader.structData.frameType = FrameType::PLAYLIST_ACTIONS;

    char txBuffer[TX_BUFFER_SIZE];

    switch(playlistAnswer->action())
    {
        case Requests::Playlist::REQUEST_PLAYLIST:
        {
            std::vector<std::string>* playlist = playlistAnswer->playlist_ptr;

            if(!playlist)
            {
                ESP_LOGE(TAG, "Playlis answer, pointer to playlis unavaliable");
                return;
            }
            answerFrameHeader.structData.actionType = (uint8_t)Requests::Playlist::REQUEST_PLAYLIST;

            printf("Playlist: ");
            for(auto it=playlist->begin(); it!=playlist->end(); it++)
            {
                printf("%s  ", (*it).c_str());
            }
            printf("\r\n");

            sendLongVector(socket, answerFrameHeader, playlist);
            //ESP_LOGI("Answer", "Send pls req answer, count: %d", playlist->size());
            sendLongVector(socket, answerFrameHeader, playlist);

            break;
        }

        case Requests::Playlist::REQUEST_PLAYLIST_POSITION:
        {
            //ESP_LOGI("Answer", "ready to answer, playlist position:%d", playlistAnswer->curPlsPos);
            answerFrameHeader.structData.actionType = (uint8_t)Requests::Playlist::REQUEST_PLAYLIST_POSITION;
            answerFrameHeader.structData.frameSize = sizeof(FrameHeader);
            answerFrameHeader.structData.data0 = playlistAnswer->curPlsPos;

            sendData(socket, answerFrameHeader.rawData, sizeof(FrameHeader));
            break;
        }

        case Requests::Playlist::CHANGE_PLAYLIST:
        {
            break;
        }

        case Requests::Playlist::CHANGE_PLAYLIST_POSITION:
        {
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

                answerFrameHeader.structData.actionType = (uint8_t)Requests::File::GET_FILE;
                answerFrameHeader.structData.frameSize = sizeof(FrameHeader) + fileName.size() + fileSize;
                answerFrameHeader.structData.data0 = fileName.size();
                answerFrameHeader.structData.data1 = fileSize;

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
                answerFrameHeader.structData.actionType = (uint8_t)Requests::File::GET_FILE;
                answerFrameHeader.structData.frameSize = sizeof(FrameHeader) + fileName.size();
                answerFrameHeader.structData.data0 = fileName.size();
                answerFrameHeader.structData.data1 = -1;

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
            result.push_back(dirPath + std::string("*"));

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
            answerFrameHeader.structData.actionType = (uint8_t)Requests::File::GET_FOLDER_CONTENT;

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
            answerFrameHeader.structData.actionType = (uint8_t)Requests::File::FILE_APPEND_DATA;
            answerFrameHeader.structData.frameSize = sizeof(FrameHeader) + fileName.size();
            answerFrameHeader.structData.data0 = fileName.size();
            answerFrameHeader.structData.data1 = fileAnswer->dataProcessed;

            char buffer[2048];
            memcpy(buffer, answerFrameHeader.rawData, sizeof(FrameHeader));
            memcpy(&buffer[sizeof(FrameHeader)], fileName.c_str(), fileName.size());
            sendData(socket, buffer, sizeof(FrameHeader)+fileName.size());
            break;
        }
    }
}

void processAnswer(NetComm::AbstractCommand* recvComm, int socket)
{
    switch (recvComm->commandType())
    {
        case NetComm::TRANSPORT_COMMAND:
        { 
            NetComm::TransportCommand* transportAnswer = static_cast<NetComm::TransportCommand*>(recvComm);
            processTransportCommand(transportAnswer, socket);
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
        default:
            break;
    }
}