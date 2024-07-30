#include "esp_log.h"
#include "esp_err.h"

#include <string.h>

#include "frameparser.hpp"
#include "filemanager/filemanager.hpp"

#include "firmware.hpp"

#include "esp_app_desc.h"

static const char *TAG = "FRAME PARSER";

FrameParser::FrameParser(int socket)
{
    m_socket = socket;

    lastRecvFrameHeader.frameType = UNDEFINED;
    lastRecvFrameHeader.frameSize = 0;
    ESP_LOGI(TAG, "new frame parser created, parent socket: %d\r\n", m_socket);
}

FrameParser::~FrameParser()
{
    ESP_LOGI(TAG, "Frame parser deleted, parent socket: %d", m_socket);
}

void FrameParser::processRecvData(uint8_t* frame, uint16_t len)
{
    for(int i=0; i<len; i++)
    {
        txBuffer.push_back(frame[i]);
    }

    while(txBuffer.size()>=sizeof(FrameHeader))
    {
        FrameHeader_uni frame_uni;
        for(int i=0; i<sizeof(FrameHeader);i++)
        {
            frame_uni.rawData[i] = txBuffer.at(i);
        }
        lastRecvFrameHeader = frame_uni.structData;

        if(txBuffer.size() >= lastRecvFrameHeader.frameSize)
        {
            lastRecvFrame.resize(lastRecvFrameHeader.frameSize);
            std::copy(txBuffer.begin(), txBuffer.begin() + lastRecvFrameHeader.frameSize, lastRecvFrame.begin());

            AbstractMessage* message = nullptr;            

            switch(lastRecvFrameHeader.frameType)
            {
                case PLAYLIST_ACTIONS:
                {
                    message = parsePlaylistActions();
                    break;
                }

                case HARDWARE_ACTIONS:
                {
                    message = parseHardwareActions();
                    break;
                }

                case FILE_ACTIONS:
                {
                    message = parseFileActions();
                    break;
                }

                case FIRMWARE_ACTIONS:
                {
                    message = parseFirmwareActions();
                    break;
                }
                default: ESP_LOGE(TAG, "Unknown frame type");
            }

            if(message) parsedMessages.push(message);

            txBuffer.erase(txBuffer.begin(), txBuffer.begin()+lastRecvFrameHeader.frameSize);
            lastRecvFrameHeader.frameSize = 0;
        }
        else
        {
            break;
        }  
    }
}

AbstractMessage* FrameParser::parseHardwareActions()
{
    switch((Requests::Hardware)lastRecvFrameHeader.action)
    {
        case Requests::Hardware::GET_SERIAL_ID: return new StringMessage(lastRecvFrame);
        case Requests::Hardware::REQUEST_PROGRESS: return new IntValueMessage(lastRecvFrame);
        case Requests::Hardware::SET_PRINT_SPEED:
        case Requests::Hardware::GET_PRINT_SPEED: return new FloatValueMessage(lastRecvFrame);
        case Requests::Hardware::SET_LED_BRIGHTNESS:
        case Requests::Hardware::GET_LED_BRIGHTNESS: return new FloatValueMessage(lastRecvFrame);
        case Requests::Hardware::SET_SCALE_COEFFICIENT:
        case Requests::Hardware::GET_SCALE_COEFFICIENT: return new FloatValueMessage(lastRecvFrame);
        case Requests::Hardware::SET_ROTATION: 
        case Requests::Hardware::GET_ROTATION: return new FloatValueMessage(lastRecvFrame);
        case Requests::Hardware::SET_CORRECTION: 
        case Requests::Hardware::GET_CORRECTION: return new FloatValueMessage(lastRecvFrame);
        case Requests::Hardware::SET_PAUSE_INTERVAL: 
        case Requests::Hardware::GET_PAUSE_INTERVAL: return new IntValueMessage(lastRecvFrame);
        case Requests::Hardware::GET_FI_GEAR2_TEETH_COUNT: return new IntValueMessage(lastRecvFrame);
        case Requests::Hardware::GET_MACHINE_MINUTES: return new IntValueMessage(lastRecvFrame);
            default: break;
    }
    return nullptr;
}

AbstractMessage* FrameParser::parsePlaylistActions()
{
    switch((Requests::Playlist)lastRecvFrameHeader.action)
    {
        case Requests::Playlist::REQUEST_PLAYLIST: return new IntValueMessage(lastRecvFrame);
        case Requests::Playlist::REQUEST_PLAYLIST_POSITION: return new IntValueMessage(lastRecvFrame);
        case Requests::Playlist::CHANGE_PLAYLIST: return new StringMessage(lastRecvFrame);
        case Requests::Playlist::CHANGE_PLAYLIST_POSITION: return new IntValueMessage(lastRecvFrame);
        case Requests::Playlist::CHANGE_PRINTNG_FILE: return new IntValueMessage(lastRecvFrame);
        case Requests::Playlist::GET_CURRENT_GALLERY: return new IntValueMessage(lastRecvFrame);
        case Requests::Playlist::SET_CURRENT_GALLERY: return new StringMessage(lastRecvFrame);
    }

    return nullptr;
}

AbstractMessage* FrameParser::parseFileActions()
{
    switch((Requests::File)lastRecvFrameHeader.action)
    {
        case Requests::File::FILE_CREATE: 
        case Requests::File::FILE_APPEND_DATA: 
        {
            while(esp_get_free_heap_size() < 1024*64) 
            {
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            return new FilePartMessage(lastRecvFrame);
        }
        
        case Requests::File::GET_FILE: return new StringMessage(lastRecvFrame);
        case Requests::File::GET_FOLDER_CONTENT: return new StringMessage(lastRecvFrame);
    }

    return nullptr;
}

AbstractMessage* FrameParser::parseFirmwareActions()
{
    switch((Requests::Firmware)lastRecvFrameHeader.action)
    {
    case Requests::Firmware::FIRMWARE_VERSION: return new StringMessage(lastRecvFrame);
    case Requests::Firmware::FIRMWARE_UPLOAD_START: 
    case Requests::Firmware::FIRMWARE_UPLOAD_PROCEED: 
    {
        while(esp_get_free_heap_size() < 1024*64) 
        {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        return new FilePartMessage(lastRecvFrame);
    }
    case Requests::Firmware::FIRMWARE_UPDATE: return new IntValueMessage(lastRecvFrame);
    case Requests::Firmware::ESP_RESTART: return new IntValueMessage(lastRecvFrame);
    }
    return nullptr;
}