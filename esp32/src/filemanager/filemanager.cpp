#include <array>

#include <vector>
#include <dirent.h>

#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"

#include "sdcard.h"

#include "filemanager.hpp"

#include "settings.hpp"

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

    // Info. Don't need in application:
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

FM_RESULT FileManager::loadGallery(std::string galleryName, uint32_t playlstPosition)
{
    currentGallery = galleryName;
    currentPlaylistPath = FileManager::mountPoint + FileManager::libraryDir + currentGallery + "/";
    return loadPlaylist(playlstPosition);
}

FM_RESULT FileManager::loadPlaylist(uint32_t playlstPosition)
{
    std::string fullFileName = currentPlaylistPath + "playlist.pls";

    FILE* playlistFile = fopen(fullFileName.c_str(), "r");

    if(playlistFile == NULL)
    {
        ESP_LOGE(TAG, "Open playlist failed, path: %s",  fullFileName.c_str());
        return FM_ERROR;
    }

    playlist.clear();

    char buf[256];
    while(fgets(buf, 256, playlistFile))
    {
        std::string resultFileName = std::string(buf);
        if(resultFileName == "\r\n") continue; // empty string
        playlist.push_back(resultFileName);
    }
    fclose(playlistFile);

    if(curPlsPos > playlist.size()-1) curPlsPos = -1;
    curPlsPos = playlstPosition-1; // first command "loadNextPrint"

    // Only Info
    
    printf("Playlist:\r\n");
    for(uint16_t i=0; i<playlist.size();i++)
    {
        printf("%s", playlist.at(i).c_str());
    }
    printf("--------\r\n");
    
    return FM_OK;
}

void FileManager::changePlaylist(const std::vector<std::string>& newPlaylist)
{
    playlist = newPlaylist;

    printf("New playlist:\r\n");
    for(uint16_t i=0; i<playlist.size();i++)
    {
        printf("%s", playlist.at(i).c_str());
    }
    printf("--------\r\n");
    
    std::string fullFileName;
    fullFileName = currentPlaylistPath + "playlist.pls";

    FILE* playlistFile = fopen(fullFileName.c_str(), "w");

    if(playlistFile == NULL)
    {
        ESP_LOGE(TAG, "Open playlist failed, res = %d");
        return;
    }

    for(auto it=playlist.begin(); it!=playlist.end(); it++)
    {
        if(*it == "\r\n") continue; // empty string
        fputs((*it).data(), playlistFile);
    }
    ESP_LOGI(TAG, "New playlist written");
    fclose(playlistFile);
}

void FileManager::changePlaylist(const std::string& newPlaylist)
{
    char* str_ptr = (char*)&newPlaylist[0];
    char* foundName;

    std::vector<std::string> newVectorPlaylist;

    while((foundName = strsep(&str_ptr, "\r\n")) != NULL)
    {  
        std::string fileName(foundName);

        if(fileName.size() == 0) continue; //empty string
        
        if(fileName.find(0xFF) == std::string::npos)
        {
            newVectorPlaylist.push_back(fileName + "\r\n");
        }
    }

    changePlaylist(newVectorPlaylist);
}

void FileManager::changePlaylistPos(int16_t newPos)
{
    if(newPos < 0)
    {
        ESP_LOGE(TAG, "New position < 0");    
        return;
    }  

    if(newPos > playlist.size()) newPos = 0;

    curPlsPos = newPos;
}

FM_RESULT FileManager::loadNextPrint()
{
    curPlsPos++;
    if(curPlsPos == playlist.size()-1) curPlsPos = 0;

    return loadPrintFromPlaylist(curPlsPos);
}

FM_RESULT FileManager::loadPrintFromPlaylist(uint16_t num)
{
    fclose(currentPrintFile);

    ESP_LOGI(TAG, "Loading next print. Pos in pls: %d", num);

    if(num > playlist.size()-1)
    {
        ESP_LOGE(TAG, "loadPrintFromPlaylis: requested num more than playlist size, num: %d, size: %d. Settling pos to 0", 
                num, playlist.size());
        num = 0;
    }

    curPlsPos = num;

    Settings::saveSetting(Settings::Digit::LAST_PLAYLIST_POSITION, curPlsPos);

    string currentFileName;
    if(playlist.size()>0)
    {
        currentFileName = playlist.at(curPlsPos);
    }
    else
    {
        ESP_LOGE(TAG,"Playlist is empty!");
        curPlsPos = -1;
        return FM_ERROR; 
    }

    std::string fullFileName;
    fullFileName = mountPoint + libraryDir + currentGallery + "/" + currentFileName;
    currentPrintFile = fopen(fullFileName.c_str(), "r");
    if(currentPrintFile == NULL)
    {
        ESP_LOGE(TAG, "Can't open print file %s", fullFileName.c_str());
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
    
    ESP_LOGI(TAG, "File %s succesfully opened. Point count: %d Printing...", currentFileName.c_str(), m_pointsNum);
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
        // size_t bytesWritten = fwrite(data_ptr, sizeof(uint8_t), dataSize, file);
        fwrite(data_ptr, sizeof(uint8_t), dataSize, file);

        long fileSize = ftell(file);

        ESP_LOGI("FileManager::fileWrite", "Part of file %s written, file size %d:", fileName.c_str(), fileSize);
        fclose(file);

        return fileSize;
    }
    else
    {
        ESP_LOGE("FileManager::fileWrite", "Error opening file to create/append: %s, acton type: %s", fileName.c_str(), writeType);
        fclose(file);
        return -1;
    }
}

void FileManager::appendFileRequest(QString filePath)
{
    requestedFiles.push(filePath);
}

FilePartMessage* FileManager::getRequestedData()
{
    if(requestedFiles.size() == 0) return nullptr;

    QString currentRequestFilePath = requestedFiles.front(); 

    if(!currentProcessingFile)
    {      
        currentProcessingFile = fopen(currentRequestFilePath.c_str(), "r");      
    }

    long filePos = 0;
    
    if(currentProcessingFile)
    {
        ESP_LOGI(TAG, "File  %s opened.", currentRequestFilePath.c_str());
        while(esp_get_free_heap_size() < 1024*64) 
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        QByteArray filePart;
        filePart.resize(FilePartMessage::defaultPartSize);
        filePos = ftell(currentProcessingFile);
        size_t bytesReaded = fread(filePart.data(), 1, FilePartMessage::defaultPartSize, currentProcessingFile);

        if(bytesReaded>0)
        {
            filePart.resize(bytesReaded);
            return new FilePartMessage(FilePartMessage::ActionType::GET_FILE, currentRequestFilePath, currentRequestFilePath, filePart, filePos);               
        }
        else
        {
            ESP_LOGI(TAG, "File sended, closing", currentRequestFilePath.c_str(), filePos);
            fclose(currentProcessingFile);
            currentProcessingFile = nullptr;
            requestedFiles.pop();
            return nullptr;
        }
    }
    else
    {
        requestedFiles.pop();
        ESP_LOGE(TAG, "File request. Can't open file %s", currentRequestFilePath.c_str());
        return new FilePartMessage(FilePartMessage::ActionType::GET_FILE, currentRequestFilePath, currentRequestFilePath, QByteArray(), -1);
    }

}
