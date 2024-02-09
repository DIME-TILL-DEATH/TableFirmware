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

            switch(lastRecvFrameHeader.frameType)
            {
                case PLAYLIST_ACTIONS:
                {
                    parsePlaylistActions();
                    break;
                }

                case TRANSPORT_ACTIONS:
                {
                    parseTransportActions();
                    break;
                }

                case FILE_ACTIONS:
                {
                    parseFileActions();
                    break;
                }

                case FIRMWARE_ACTIONS:
                {
                    parseFirmwareActions();
                    break;
                }
                default: ESP_LOGE(TAG, "Unknown frame type");
            }

            txBuffer.erase(txBuffer.begin(), txBuffer.begin()+lastRecvFrameHeader.frameSize);
            lastRecvFrameHeader.frameSize = 0;
        }
        else
        {
            break;
        }  
    }
}

void FrameParser::parseTransportActions()
{
    NetComm::TransportCommand* command = new NetComm::TransportCommand(0, (Requests::Transport)lastRecvFrameHeader.action);
    lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin()+sizeof(FrameHeader));
    switch((Requests::Transport)lastRecvFrameHeader.action)
    {
        case Requests::Transport::SET_PRINT_SPEED:
        {
            command->printSpeed = (float_t)lastRecvFrameHeader.data0;
            break;
        }
        default: {};
    }
    parsedCommands.push_back(command);
}

#define PLAYLIST_RX_BUFFER 512
void FrameParser::parsePlaylistActions()
{
    NetComm::PlaylistCommand* command = new NetComm::PlaylistCommand(0, (Requests::Playlist)lastRecvFrameHeader.action);

    lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin()+sizeof(FrameHeader));
    switch((Requests::Playlist)lastRecvFrameHeader.action)
    {
        case Requests::Playlist::REQUEST_PLAYLIST:
        {
            //ESP_LOGI(TAG, "Playlist request formed");
            break;
        }

        case Requests::Playlist::REQUEST_PLAYLIST_POSITION:
        {
            //ESP_LOGI(TAG, "Playlist request position formed");
            break;    
        }

        case Requests::Playlist::CHANGE_PLAYLIST:
        {
            char buffer[PLAYLIST_RX_BUFFER*2];
            memset(buffer, 0XFF, PLAYLIST_RX_BUFFER*2);

            std::vector<std::string>* newPlaylist = new std::vector<std::string>;
            int charRemainded=0;
            while(lastRecvFrame.size()>0)
            {
                int partSize;
                if(lastRecvFrame.size()%PLAYLIST_RX_BUFFER == 0)
                {
                    partSize = PLAYLIST_RX_BUFFER;
                }
                else
                {
                    partSize = lastRecvFrame.size()%PLAYLIST_RX_BUFFER;
                }

                memcpy(&buffer[charRemainded], lastRecvFrame.data(), partSize);
                lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin()+partSize);

                char* str_ptr = &buffer[0];
                char* foundName;
                size_t charsCopied = 0;

                int delimitersCount = 0;
                while((foundName = strsep(&str_ptr, "|")) != NULL)
                {  
                    std::string fileName(foundName);
                    
                    if(fileName.find(0xFF) == std::string::npos)
                    {
                        newPlaylist->push_back(fileName + "\r\n");
                        charsCopied += strlen(foundName);
                        delimitersCount++;
                    }

                }
                charRemainded = charRemainded + partSize - charsCopied - delimitersCount;

                memcpy(&buffer[0], &buffer[charsCopied+delimitersCount], charRemainded);
                memset(&buffer[charRemainded], 0XFF, PLAYLIST_RX_BUFFER*2-charRemainded);
            }

            command->playlist_ptr = newPlaylist;
            break;
        }

        case Requests::Playlist::CHANGE_PLAYLIST_POSITION:
        {
            // Same actions
        }

        case Requests::Playlist::CHANGE_PRINTNG_FILE:
        {
            command->curPlsPos = lastRecvFrameHeader.data0;
            break;
        }
    }
    if(command) parsedCommands.push_back(command);
}

void FrameParser::parseFileActions()
{
    char buffer[512];
    memset(buffer, 0, 512);

    NetComm::FileCommand* command = new NetComm::FileCommand(0, (Requests::File)lastRecvFrameHeader.action);

    uint16_t pathStringSize = lastRecvFrameHeader.frameParameters;
    int32_t partSize = lastRecvFrameHeader.data0;

    lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin()+sizeof(FrameHeader));
    memcpy(buffer, lastRecvFrame.data(), pathStringSize);

    std::string fullFileName = std::string(buffer);
    command->path = fullFileName;

    lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin() + pathStringSize);

    switch((Requests::File)lastRecvFrameHeader.action)
    {
        case Requests::File::FILE_CREATE:
        {
            command->dataProcessed = FileManager::fileWrite(fullFileName, "w", lastRecvFrame.data(), partSize);
            break;
        }

        case Requests::File::FILE_APPEND_DATA:
        {
            command->dataProcessed = FileManager::fileWrite(fullFileName, "a", lastRecvFrame.data(), partSize);
            break;
        }
        default: {}
    }

    if(command) parsedCommands.push_back(command);
}

void FrameParser::parseFirmwareActions()
{
    NetComm::FirmwareCommand* command = new NetComm::FirmwareCommand(0, (Requests::Firmware)lastRecvFrameHeader.action);
    command->fileSize = lastRecvFrameHeader.data1;
    lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin()+sizeof(FrameHeader));
    std::string fullFileName = FileManager::mountPoint + "firmware.bin";

    int32_t partSize = lastRecvFrameHeader.data0;

    switch((Requests::Firmware)lastRecvFrameHeader.action)
    {
    case Requests::Firmware::FIRMWARE_VERSION:
    {
        const esp_app_desc_t* appDesc = esp_app_get_description();
        command->firmwareVersion = appDesc->version;
        break;
    }

    case Requests::Firmware::FIRMWARE_UPLOAD_START:
    {   
        command->dataProcessed = FileManager::fileWrite(fullFileName, "wb", lastRecvFrame.data(), partSize);
        break;
    }

    case Requests::Firmware::FIRMWARE_UPLOAD_PROCEED:
    {
        command->dataProcessed = FileManager::fileWrite(fullFileName, "ab", lastRecvFrame.data(), partSize);
        break;
    }

    case Requests::Firmware::FIRMWARE_UPDATE:
    {
        FW_DoFirmwareUpdate();
        break;
    }
    }
    if(command) parsedCommands.push_back(command);
}