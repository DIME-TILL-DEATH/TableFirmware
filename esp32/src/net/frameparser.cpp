#include "esp_log.h"

#include <string.h>

#include "frameparser.hpp"

static const char *TAG = "FRAME PARSER";

FrameParser::FrameParser(int socket)
{
    m_socket = socket;
    lastRecvFrameHeader.frameSize = 0;
    ESP_LOGI(TAG, "new frame parser created, parent socket: %d\r\n", m_socket);
}

void FrameParser::processRecvData(uint8_t* frame, uint16_t len)
{
    for(int i=0; i<len; i++)
    {
        recvData.push_back(frame[i]);
    }

    while(recvData.size()>=sizeof(FrameHeader))
    {
        FrameHeader_uni frame_uni;
        for(int i=0; i<sizeof(FrameHeader);i++)
        {
            frame_uni.rawData[i] = recvData.at(i);
        }
        lastRecvFrameHeader = frame_uni.structData;

        if(recvData.size() >= lastRecvFrameHeader.frameSize)
        {
            switch(lastRecvFrameHeader.frameType)
            {
                case PLAYLIST_ACTIONS:
                {
                    m_command = new NetComm::PlaylistCommand(0, (NetComm::PlaylistCommand::PlaylistActions)lastRecvFrameHeader.actionType);
                    break;
                }

                case TRANSPORT_ACTIONS:
                {
                    m_command = new NetComm::TransportCommand(0, (NetComm::TransportCommand::TransportActions)lastRecvFrameHeader.actionType);
                    break;
                }
                default: ESP_LOGE(TAG, "Unknown frame type");
            }

            parsedCommands.push_back(m_command);
        }

        recvData.erase(recvData.begin(), recvData.begin()+lastRecvFrameHeader.frameSize);
        lastRecvFrameHeader.frameSize = 0;
    }
}