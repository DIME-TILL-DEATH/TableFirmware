#include "esp_log.h"

#include "frameparser.hpp"

static const char *TAG = "FRAME PARSER";

FrameParser::FrameParser(int socket)
{
    m_socket = socket;
    ESP_LOGI(TAG, "new frame parser created, parent socket: %d\r\n", m_socket);
}

void FrameParser::processRecvData(uint8_t* frame, uint16_t len)
{
    FrameHeader* frameHeader_ptr = (FrameHeader*)frame;

    switch(frameHeader_ptr->frameType)
    {
        case PLAYLIST_ACTIONS:
        {
            
            break;
        }

        case TRANSPORT_ACTIONS:
        {
            m_command = new NetComm::TransportCommand(0, (NetComm::TransportCommand::TransportActions)frameHeader_ptr->actionType);
            break;
        }
        default: ESP_LOGE(TAG, "Unknown frame type");
    }
}