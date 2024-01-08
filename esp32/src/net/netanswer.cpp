#include "netanswer.hpp"

#include <string.h>
#include <vector>
#include "esp_log.h"

#include "frames.h"

void processTransportCommand(NetComm::TransportCommand* transportAnswer, uint8_t* txBuffer, size_t* answerLen)
{
    FrameHeader_uni answerFrame;
 
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
            *answerLen = sizeof(FrameHeader); 
            break;
        }

        case NetComm::TransportCommand::SET_PRINT:
        {
            break;
        }
    }
}

void processPlaylistCommand(NetComm::PlaylistCommand* playlistAnswer, uint8_t* txBuffer, size_t* answerLen)
{   
    switch(playlistAnswer->action())
    {
        case NetComm::PlaylistCommand::REQUEST_PLAYLIST:
        {
            ESP_LOGI("Answer", "ready to answer playlist");
            break;
        }

        case NetComm::PlaylistCommand::CHANGE_PLAYLIST:
        {
            break;
        }
    }
}

void formAnswer(NetComm::AbstractCommand* recvComm, uint8_t* txBuffer, size_t* answerLen)
{
    switch (recvComm->commandType())
    {
        case NetComm::TRANSPORT_COMMAND:
        { 
            NetComm::TransportCommand* transportAnswer = static_cast<NetComm::TransportCommand*>(recvComm);
            processTransportCommand(transportAnswer, txBuffer, answerLen);
            break;
        }
        case NetComm::PLAYLIST_COMMAND:
        {
            NetComm::PlaylistCommand* playlistAnswer = static_cast<NetComm::PlaylistCommand*>(recvComm);
            processPlaylistCommand(playlistAnswer, txBuffer, answerLen);          
            break;
        }
        default:
            break;
    }
}