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

static const char *FM_TAG = "FILE MANGER";

FileManager::FileManager()
{
    SDBUS_Init();
}

FM_RESULT FileManager::connectSDCard()
{
    // FRESULT res;

    esp_err_t retEsp = SDCARD_Mount();
    if(retEsp != ESP_OK)
    {
        return FM_ERROR;
    }

    DIR* dir;
    dir = opendir(MOUNT_POINT"/");
    if(dir == NULL)
    {
        ESP_LOGE(FM_TAG, "f_opendir() failed");
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
            printf("    DIR===%s\n", entry->d_name);
            totalDirs++;
        }
    }
    closedir(dir);

    printf("(total: %lu dirs, %lu files)--------\r\n",
                totalDirs, totalFiles);

    return FM_OK;
}

FM_RESULT FileManager::loadPlaylist()
{
    FILE* playlistFile;
    playlistFile = fopen(MOUNT_POINT"/playlist.pls", "r");

    if(playlistFile == NULL)
    {
        ESP_LOGE(FM_TAG, "Open playlist failed, res = %d");
        return FM_ERROR;
    }

    playlist.clear();

    char buf[256];
    while(fgets(buf, 256, playlistFile))
    {
        playlist.push_back(std::string(buf));
    }
    fclose(playlistFile);

    curPlsPos = -1; // no file loaded

    printf("Playlist:\r\n");
    for(uint16_t i=0; i<playlist.size();i++)
    {
        printf("%s\r\n", playlist.at(i).c_str());
    }
    printf("--------\r\n");

    return FM_OK;
}

FM_RESULT FileManager::loadNextPrint()
{
    fclose(currentPrintFile); // close previous

    curPlsPos++;
    if(curPlsPos == playlist.size()) curPlsPos = 0;

    string currentFileName;
    if(playlist.size()>0)
    {
        currentFileName = playlist.at(curPlsPos);
    }

    std::string fullFileName;
    fullFileName = std::string(MOUNT_POINT"/") + currentFileName;
    currentPrintFile = fopen(fullFileName.c_str(), "r");
    if(currentPrintFile == NULL)
    {
        ESP_LOGE(FM_TAG, "Can't open print file %s", currentFileName.c_str());
        return FM_ERROR;
    }

    char* result;
    char readBuf[512];
    m_pointsNum = 0;
    do
    {
        result = fgets(readBuf, 512, currentPrintFile);
        m_pointsNum++;
    } while (result);
    fseek(currentPrintFile, 0, SEEK_SET);
    
    ESP_LOGI(FM_TAG, "File %s succesfully opened. Point num: %d Printing...", currentFileName.c_str(), m_pointsNum);

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
