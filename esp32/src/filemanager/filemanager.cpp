#include <array>

#include <vector>
#include <dirent.h>

#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"

#include "sdcard.h"

#include "filemanager.hpp"

using namespace std;

FileManager::FileManager()
{
    SDBUS_Init();
}

FM_RESULT FileManager::connectSDCard()
{
    esp_err_t retEsp = SDCARD_Mount();
    if(retEsp != ESP_OK)
    {
        return FM_ERROR;
    }

    // Info. Don't need whth application:
    /*
    DIR* dir;
    dir = opendir(MOUNT_POINT"/");
    if(dir == NULL)
    {
        ESP_LOGE(TAG, "opendir() failed");
        return FM_ERROR;
    }

    dirent *entry;

    uint32_t totalFiles = 0;
    uint32_t totalDirs = 0;
    printf("--------\r\nRoot directory:\r\n");

    while ((entry = readdir(dir)) != NULL) 
    {
        if(entry->d_type == DT_REG)
        {
            printf("    %s\n", entry->d_name);
            totalFiles++;
        }
        else if(entry->d_type == DT_DIR)
        {
            printf("    DIR=%s\n", entry->d_name);
            totalDirs++;
        }
    }
    closedir(dir);

    printf("(total: %lu dirs, %lu files)--------\r\n",
                totalDirs, totalFiles);*/

    return FM_OK;
}

FM_RESULT FileManager::loadPlaylist(std::string playlistName, uint32_t playlstPosition)
{
    std::string fullFileName;
    fullFileName = mountPoint + playlistsDir + playlistName;

    FILE* playlistFile = fopen(fullFileName.c_str(), "r");

    if(playlistFile == NULL)
    {
        ESP_LOGE(TAG, "Open playlist failed, res = %d");
        return FM_ERROR;
    }

    playlist.clear();

    char buf[256];
    while(fgets(buf, 256, playlistFile))
    {
        playlist.push_back(std::string(buf));
    }
    fclose(playlistFile);

    curPlsPos = playlstPosition-1; // first command "loadNextPrint"

    // Only Info
    /*
    printf("Playlist:\r\n");
    for(uint16_t i=0; i<playlist.size();i++)
    {
        printf("%s\r\n", playlist.at(i).c_str());
    }
    printf("--------\r\n");
    */

    return FM_OK;
}

void FileManager::changePlaylist(const std::vector<std::string>* newPlaylist)
{
    playlist = *newPlaylist;
    
    std::string fullFileName;
    fullFileName = mountPoint + playlistsDir + "playlist.pls";

    FILE* playlistFile = fopen(fullFileName.c_str(), "w");

    if(playlistFile == NULL)
    {
        ESP_LOGE(TAG, "Open playlist failed, res = %d");
        return;
    }

    for(auto it=playlist.begin(); it!=playlist.end(); it++)
    {
        fputs((*it).data(), playlistFile);
        //printf("%s  ", (*it).c_str());
    }
    ESP_LOGI(TAG, "New playlist wirtten");
    fclose(playlistFile);
}

void FileManager::changePlaylistPos(int16_t newPos)
{
    if(newPos > -1) curPlsPos = newPos;
}

FM_RESULT FileManager::loadNextPrint()
{
    curPlsPos++;
    if(curPlsPos == playlist.size()) curPlsPos = 0;

    loadPrintFromPlaylist(curPlsPos);

    return FM_OK;
}

FM_RESULT FileManager::loadPrintFromPlaylist(uint16_t num)
{
    fclose(currentPrintFile); // close previous

    if(num > playlist.size()-1)
    {
        ESP_LOGE(TAG, "loadPrintFromPlaylis: requested num more than playlist size, num: %d, size: %d", num, playlist.size());
    }

    curPlsPos = num;

    string currentFileName;
    if(playlist.size()>0)
    {
        currentFileName = playlist.at(curPlsPos);
    }

    std::string fullFileName;
    fullFileName = mountPoint + libraryDir + currentFileName;
    currentPrintFile = fopen(fullFileName.c_str(), "r");
    if(currentPrintFile == NULL)
    {
        ESP_LOGE(TAG, "Can't open print file %s", currentFileName.c_str());
        return FM_ERROR;
    }

     ESP_LOGE(TAG, "File opened");

    char* result;
    char readBuf[512];
    m_pointsNum = 0;
    ESP_LOGE(TAG, "Counting points %d", m_pointsNum);
    do
    {
        result = fgets(readBuf, 512, currentPrintFile);
        m_pointsNum++;
         
    } while (result);
    fseek(currentPrintFile, 0, SEEK_SET);
    ESP_LOGE(TAG, "Finish ounting points %d", m_pointsNum);
    
    ESP_LOGI(TAG, "File %s succesfully opened. Point num: %d Printing...", currentFileName.c_str(), m_pointsNum);
    return FM_OK;
}

GCode::GAbstractComm* FileManager::readNextComm()
{
    char readBuf[256];
    char* result;
    
    if(currentPrintFile == nullptr)
    {
        return nullptr;
    }

    result = fgets(readBuf, 256, currentPrintFile);
    if(result)
    {
        GCode::GAbstractComm* answer = nullptr;
        vector<string> strArgs;

        char* line_ptr = &readBuf[0];
        char* foundArg;
        while((foundArg = strsep(&line_ptr, " ")) != NULL)
        {
            strArgs.push_back(foundArg);
        }

        if(strArgs.size()>0)
        {
            // forming commands:
            if(strArgs.at(0) == string("M51"))
            {
                GCode::M51Comm* command = new GCode::M51Comm(strArgs.at(1));
                answer = command;
            }
            else if(strArgs.at(0) == "G1")
            {
                string strValue = strArgs.at(1);
                strValue.erase(0, 1);
                float_t x = stof(strValue);

                strValue = strArgs.at(2);
                strValue.erase(0, 1);
                float_t y = stof(strValue);;

                strValue = strArgs.at(3);
                strValue.erase(0, 1);
                float_t speed = stof(strValue);;
                GCode::G1Comm* command = new GCode::G1Comm({x, y}, speed);
                answer = command;
            }
            else if(strArgs.at(0) == string("G4"))
            {
                string strValue = strArgs.at(1);
                strValue.erase(0, 1);
                uint32_t value = stoi(strValue);

                GCode::G4Comm* command = new GCode::G4Comm(value);
                answer = command;
            }
        }
        return answer;
    }
    else return nullptr;
}

int32_t FileManager::fileWrite(std::string fileName, const char* writeType, void* data_ptr, size_t dataSize)
{
    FILE* file = fopen(fileName.c_str(), writeType);
    if(file != NULL)
    {
        size_t bytesWritten = fwrite(data_ptr, sizeof(uint8_t), dataSize, file);

        ESP_LOGI("FileManager::fileWrite", "Part of file %s written, part size %d:", fileName.c_str(), bytesWritten);
        fclose(file);

        return bytesWritten;
    }
    else
    {
        ESP_LOGE("FileManager::fileWrite", "Error opening file to append %s", fileName.c_str());
        return -1;
    }
}
