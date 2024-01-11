#include "netanswer.hpp"

#include <string.h>
#include <vector>
#include "esp_log.h"

#include "frames.h"

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
    char txBuffer[128];
 
    switch(transportAnswer->action())
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
            ESP_LOGI("Answer", "ready to answer, Cur point: %d, All points: %d", transportAnswer->progress.currentPoint, transportAnswer->progress.printPoints);
            answerFrame.structData.frameType = FrameType::TRANSPORT_ACTIONS;
            answerFrame.structData.actionType = NetComm::TransportCommand::REQUEST_PROGRESS;
            answerFrame.structData.frameSize = sizeof(FrameHeader);

            answerFrame.structData.data0 = transportAnswer->progress.currentPoint;    
            answerFrame.structData.data1 = transportAnswer->progress.printPoints; 
            memcpy(txBuffer, answerFrame.rawData, sizeof(FrameHeader));  

            sendData(socket, txBuffer, sizeof(FrameHeader));
            break;
        }

        case NetComm::TransportCommand::SET_PRINT:
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
    char txBuffer[TX_PLAYLIST_BUFFER_SIZE];

    switch(playlistAnswer->action())
    {
        case NetComm::PlaylistCommand::REQUEST_PLAYLIST:
        {
            uint16_t len = 0;
            std::vector<std::string>* playlist = playlistAnswer->playlist_ptr;

            len += sizeof(FrameHeader);
            for(auto it = playlist->begin(); it!=playlist->end(); ++it)
            {
                len += (*it).size();
            }

            answerFrameHeader.structData.frameType = FrameType::PLAYLIST_ACTIONS;
            answerFrameHeader.structData.actionType = NetComm::PlaylistCommand::REQUEST_PLAYLIST;
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

        case NetComm::PlaylistCommand::CHANGE_PLAYLIST:
        {
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
        default:
            break;
    }
}