#include "esp_log.h"
#include "esp_err.h"

#include <string.h>

#include "frameparser.hpp"
#include "filemanager/filemanager.hpp"

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
    NetComm::TransportCommand* command = new NetComm::TransportCommand(0, (Requests::Transport)lastRecvFrameHeader.actionType);
    ESP_LOGI(TAG, "Formed new transport req, act type: %d", lastRecvFrameHeader.actionType);
    parsedCommands.push_back(command);
}

#define PLAYLIST_RX_BUFFER 512
void FrameParser::parsePlaylistActions()
{
    NetComm::PlaylistCommand* command = new NetComm::PlaylistCommand(0, (Requests::Playlist)lastRecvFrameHeader.actionType);
    switch((Requests::Playlist)lastRecvFrameHeader.actionType)
    {
        case Requests::Playlist::REQUEST_PLAYLIST:
        {
            ESP_LOGI(TAG, "Playlist request formed");
            break;
        }

        case Requests::Playlist::REQUEST_PLAYLIST_POSITION:
        {
            ESP_LOGI(TAG, "Playlist request position formed");
            break;    
        }

        case Requests::Playlist::CHANGE_PLAYLIST:
        {
            lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin()+sizeof(FrameHeader));

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

                // ESP_LOGI(TAG, "Buffer part, size: %d", lastRecvFrame.size());
                memcpy(&buffer[charRemainded], lastRecvFrame.data(), partSize);
                lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin()+partSize);
                // ESP_LOGI(TAG, "splitted, size: %d", lastRecvFrame.size());

                char* str_ptr = &buffer[0];
                char* foundName;
                size_t charsCopied = 0;

                // printf("analyse:");
                // for(int i=0; i<PLAYLIST_RX_BUFFER*2; i++)
                //     printf("%c", buffer[i]);

                // printf("\r\n");
                int delimitersCount = 0;
                while((foundName = strsep(&str_ptr, "|")) != NULL)
                {
                    // ESP_LOGI(TAG, "1");
                    
                    std::string fileName(foundName);
                    
                    if(fileName.find(0xFF) == std::string::npos)
                    {
                        // ESP_LOGI(TAG, "Finded name: %s", foundName);
                        newPlaylist->push_back(fileName + "\r\n");
                        charsCopied += strlen(foundName);
                        delimitersCount++;
                    }
                    // else
                    // {
                    //     ESP_LOGI(TAG, "Dropped");
                    // }
                }
                charRemainded = charRemainded + partSize - charsCopied - delimitersCount;

                // ESP_LOGI(TAG, "Chars copied: %d chars remainded: %d", charsCopied, charRemainded);
                memcpy(&buffer[0], &buffer[charsCopied+delimitersCount], charRemainded);
                memset(&buffer[charRemainded], 0XFF, PLAYLIST_RX_BUFFER*2-charRemainded);
            }

            command->playlist_ptr = newPlaylist;
            break;
        }

        case Requests::Playlist::CHANGE_PLAYLIST_POSITION:
        {
            lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin()+sizeof(FrameHeader));
            int16_t plsPosition = (lastRecvFrame.at(0)<<0) | (lastRecvFrame.at(1)<<8);

            command->curPlsPos = plsPosition;
            break;
        }
    }
    if(command) parsedCommands.push_back(command);
}

void FrameParser::parseFileActions()
{
    switch((Requests::File)lastRecvFrameHeader.actionType)
    {
        case Requests::File::GET_FILE:
        {
            char buffer[1024];
            memset(buffer, 0, 1024);

            lastRecvFrame.erase(lastRecvFrame.begin(), lastRecvFrame.begin()+sizeof(FrameHeader));
            memcpy(buffer, lastRecvFrame.data(), lastRecvFrame.size());

            NetComm::FileCommand* command = new NetComm::FileCommand(0, (Requests::File)lastRecvFrameHeader.actionType);
            command->fileName = FileManager::mountPoint + std::string(buffer);

            if(command) parsedCommands.push_back(command);

            break;
        }
    }
}