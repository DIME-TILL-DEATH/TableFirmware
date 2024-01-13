#include "netanswer.hpp"

#include <string.h>
#include <vector>
#include "esp_log.h"

#include "ff.h"

#include "filemanager/filemanager.hpp"
#include "frames.h"
#include "requestactions.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

static const char *TAG = "NET ANSWER";

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

void processTransportCommand(NetComm::TransportCommand* transportAnswer, int socket)
{
    FrameHeader_uni answerFrame;
    memset(answerFrame.rawData, 0, sizeof(FrameHeader));
    answerFrame.structData.frameType = FrameType::TRANSPORT_ACTIONS;

    char txBuffer[128];
 
    switch(transportAnswer->action())
    {
        case Requests::Transport::PREVIOUS_PRINT:
        {
            break;
        }

        case Requests::Transport::PAUSE_PRINTING:
        {
            
            break;
        }

        case Requests::Transport::NEXT_PRINT:
        {
            break;
        }

        case Requests::Transport::REQUEST_PROGRESS:
        {
            ESP_LOGI("Answer", "ready to answer, Cur point: %d, All points: %d", transportAnswer->progress.currentPoint, transportAnswer->progress.printPoints);
            
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

#define TX_PLAYLIST_BUFFER_SIZE 1024
void processPlaylistCommand(NetComm::PlaylistCommand* playlistAnswer, int socket)
{   
    FrameHeader_uni answerFrameHeader;
    memset(answerFrameHeader.rawData, 0, sizeof(FrameHeader));
    answerFrameHeader.structData.frameType = FrameType::PLAYLIST_ACTIONS;

    char txBuffer[TX_PLAYLIST_BUFFER_SIZE];

    switch(playlistAnswer->action())
    {
        case Requests::Playlist::REQUEST_PLAYLIST:
        {
            uint16_t len = 0;
            std::vector<std::string>* playlist = playlistAnswer->playlist_ptr;

            if(!playlist)
            {
                ESP_LOGE(TAG, "Playlis answer, pointer to playlis unavaliable");
                return;
            }

            len += sizeof(FrameHeader);
            for(auto it = playlist->begin(); it!=playlist->end(); ++it)
            {
                len += (*it).size();
            }

            answerFrameHeader.structData.actionType = (uint8_t)Requests::Playlist::REQUEST_PLAYLIST;
            answerFrameHeader.structData.frameSize = len;
            
            int parts = len / TX_PLAYLIST_BUFFER_SIZE + 1;
            ESP_LOGI("Answer", "ready to answer playlist, frame len: %d, parts: %d", len, parts);
            
            memcpy(txBuffer, answerFrameHeader.rawData, sizeof(FrameHeader));
            auto curItem = playlist->begin();
            int curSendBytes = sizeof(FrameHeader);

            for(int i=0; i<parts; i++)
            {
                while((curItem != playlist->end()) && ((curSendBytes + (*curItem).size()) < TX_PLAYLIST_BUFFER_SIZE))
                {
                    memcpy(&txBuffer[curSendBytes], (*curItem).data(), (*curItem).size());
                    curSendBytes += (*curItem).size();
                    curItem++;
                }
                ESP_LOGI("Answer", "Send part: %d, len: %d", i, curSendBytes);
                sendData(socket, txBuffer, curSendBytes);
                curSendBytes = 0;
            }
            break;
        }

        case Requests::Playlist::REQUEST_PLAYLIST_POSITION:
        {
            ESP_LOGI("Answer", "ready to answer, playlist position:%d", playlistAnswer->curPlsPos);
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
            std::string fileName = fileAnswer->fileName;
            FILE* reqFile = fopen(fileName.c_str(), "r");
            if(reqFile)
            {
                fseek(reqFile, 0 , SEEK_END);
                long fileSize = ftell(reqFile);
                ESP_LOGI(TAG, "File  %s opened. Size: %d", fileName.c_str(), fileSize);
                fseek(reqFile, 0 , SEEK_SET); 

                fileName.erase(0, FileManager::mountPoint.size());

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
                    // ESP_LOGI(TAG, "Bytes sended: %d", bytesReaded);
                }while(bytesReaded>0);

                fclose(reqFile);
            }
            else
            {
                ESP_LOGE(TAG, "File request. Can't open print file %s", fileName.c_str());
                return;
            }

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