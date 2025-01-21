#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"

#include "settings.hpp"
#include "filemanager.hpp"

float_t Settings::getSetting(Settings::Digit setting)
{
    float_t settingValue=defaultSetting(setting);;
    char buf[512];

    std::string settingRequest = settingName(setting) + "=" + "%f";

    std::string fullFileName;
    fullFileName = FileManager::mountPoint + "settings.ini";
    FILE* file = fopen(fullFileName.c_str(), "r");
    if(file == NULL)
    {
        ESP_LOGE(TAG, "getSetting error. Can't open settings file. Return default value.");
        goto ENDING;
    }

    while(xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        ESP_LOGW(TAG, "SPI mutex taken by file manager, wait(get setting, digit)");
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    while(fgets(buf, 512, file))
    {
        int result = sscanf(buf, settingRequest.c_str(), &settingValue);
        if(result>0)
        {
            printf("Searcing setting '%s', founded items %d, result: %f\r\n", settingRequest.c_str(), result, settingValue);
            goto ENDING;
        }
    }
    
    printf("Setting '%s' not found. Return default value\r\n", settingRequest.c_str());
    
    ENDING:
    xSemaphoreGive(spiMutex);
    fclose(file);
    return settingValue;
}

std::string Settings::getSetting(Settings::String setting)
{
    std::string settingValue=defaultSetting(setting);;
    char buf[512];

    std::string settingRequest = settingName(setting) + "=" + "%s";

    std::string fullFileName;
    fullFileName = FileManager::mountPoint + "settings.ini";
    FILE* file = fopen(fullFileName.c_str(), "r");
    if(file == NULL)
    {
        ESP_LOGE(TAG, "getSetting error. Can't open settings file. Return default value.");
        goto ENDING;
    }

    while(xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        ESP_LOGW(TAG, "SPI mutex taken by file manager, wait(getSetting, string)");
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    while(fgets(buf, 512, file))
    {
        char readResult[512];
        int result = sscanf(buf, settingRequest.c_str(), readResult);
        if(result>0)
        {
            settingValue = std::string(readResult);
            printf("Searcing setting '%s', founded items %d, result: %s\r\n", settingRequest.c_str(), result, settingValue.c_str());
            goto ENDING;
        }
    }
    
    printf("Setting '%s' not found. Return default value\r\n", settingRequest.c_str());
    
    ENDING:
    xSemaphoreGive(spiMutex);
    fclose(file);
    return settingValue;
}

void Settings::saveSetting(Settings::Digit setting, float_t value)
{
    std::string originalFilePath = FileManager::mountPoint + "settings.ini";
    FILE* originalfile = fopen(originalFilePath.c_str(), "r");
    if(originalfile == NULL)
    {
        ESP_LOGE(TAG, "SaveSetting error. Can't open original file");
    }

    std::string tmpFilePath = FileManager::mountPoint + "replace.tmp";
    FILE* tempFile = fopen(tmpFilePath.c_str(), "w");
    if(tempFile == NULL)
    {
        ESP_LOGE(TAG, "SaveSetting error. Can't open temp file for write");
        return;
    }
    char buf[512];
    bool settingFinded = false;

    while(xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        ESP_LOGW(TAG, "SPI mutex taken by file manager, wait(saveSetting, digit)");
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    while(fgets(buf, 512, originalfile))
    {
        std::string readResult(buf);
        if(readResult.compare(0, settingName(setting).size(), settingName(setting)) == 0)
        {
            std::string resultSettingString = settingName(setting) + "=" + std::to_string(value) + "\n";
            fputs(resultSettingString.c_str(), tempFile);
            settingFinded=true;
        }
        else
        {
            fputs(buf, tempFile);
        }
    }
    
    if(!settingFinded)
    {
        std::string resultSettingString = settingName(setting) + "=" + std::to_string(value) + "\n";
        fputs(resultSettingString.c_str(), tempFile);
        settingFinded=true;
    }

    xSemaphoreGive(spiMutex);

    fclose(originalfile);
    fclose(tempFile);

    remove(originalFilePath.c_str());
    rename(tmpFilePath.c_str(), originalFilePath.c_str());
}

void Settings::saveSetting(Settings::String setting, std::string value)
{
    std::string originalFilePath = FileManager::mountPoint + "settings.ini";
    FILE* originalfile = NULL;
    do{ 
        originalfile = fopen(originalFilePath.c_str(), "r");

        if(originalfile == NULL)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            ESP_LOGE(TAG, "SaveSetting error. Can't open original file for read");
        }
    }while(originalfile == NULL);

    std::string tmpFilePath = FileManager::mountPoint + "replace.tmp";
    FILE* tempFile = fopen(tmpFilePath.c_str(), "w");
    if(tempFile == NULL)
    {
        ESP_LOGE(TAG, "SaveSetting error. Can't open temp file for write");
        return;
    }

    char buf[512];
    bool settingFinded = false;

    while(xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        ESP_LOGW(TAG, "SPI mutex taken by file manager, wait(saveSetting, string)");
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    while(fgets(buf, 512, originalfile))
    {
        std::string readResult(buf);
        if(readResult.compare(0, settingName(setting).size(), settingName(setting)) == 0)
        {
            std::string resultSettingString = settingName(setting) + "=" + value;
            fputs(resultSettingString.c_str(), tempFile);
            fputs("\n", tempFile);
            settingFinded = true;
        }
        else
        {
            fputs(buf, tempFile);
        }
    }

    if(!settingFinded)
    {
        std::string resultSettingString = settingName(setting) + "=" + value;
        fputs(resultSettingString.c_str(), tempFile);
        fputs("\n", tempFile);
    }

    xSemaphoreGive(spiMutex);
    
    
    fclose(originalfile);
    fclose(tempFile);

    remove(originalFilePath.c_str());
    rename(tmpFilePath.c_str(), originalFilePath.c_str());
}

std::string Settings::settingName(Settings::Digit settingType)
{
    switch(settingType)
    {
        case Settings::Digit::PRINT_ROTATION: return "PRINT_ROTATION";
        case Settings::Digit::PRINT_SPEED: return "PRINT_SPEED";
        case Settings::Digit::SCALE_COEF: return "SCALE_COEF";
        case Settings::Digit::LAST_PLAYLIST_POSITION: return "LAST_PLAYLIST_POSITION";
        case Settings::Digit::LED_BRIGHTNESS: return "LED_BRIGHTNESS";
        case Settings::Digit::CORRETION_LENGTH: return "CORRECTION_LENGTH";
        case Settings::Digit::PAUSE_INTERVAL: return "PAUSE_INTERVAL";
        case Settings::Digit::FI_GEAR2_TEETH_COUNT: return "FI_GEAR2_TEETH_COUNT";
        case Settings::Digit::MACHINE_MINUTES: return "MACHINE_MINUTES";
    }
    ESP_LOGE(TAG, "Unknown setting type!");
    return "";
}

float_t Settings::defaultSetting(Settings::Digit settingType)
{
    switch(settingType)
    {
        case Settings::Digit::PRINT_ROTATION: return 0;
        case Settings::Digit::PRINT_SPEED: return 25;
        case Settings::Digit::SCALE_COEF: return 1.0;
        case Settings::Digit::LAST_PLAYLIST_POSITION: return 0;
        case Settings::Digit::LED_BRIGHTNESS: return 0.5;
        case Settings::Digit::CORRETION_LENGTH: return 0;
        case Settings::Digit::PAUSE_INTERVAL: return 1000;
        case Settings::Digit::FI_GEAR2_TEETH_COUNT: return 160;
        case Settings::Digit::MACHINE_MINUTES: return 0;
    }
    ESP_LOGE(TAG, "Unknown setting type!");
    return 0;
}

std::string Settings::settingName(Settings::String settingType)
{
    switch(settingType)
    {
        case Settings::String::SERIAL_ID: return "SERIAL_ID";
        case Settings::String::PLAYLIST: return "PLAYLIST";
        case Settings::String::PRINT_GALLERY: return "PRINT_GALLERY";
        case Settings::String::WIFI_SSID: return "WIFI_SSID";
        case Settings::String::WIFI_PASSWORD: return "WIFI_PASSWORD";
        case Settings::String::LED_TYPE: return "LED_TYPE";
    }
    ESP_LOGE(TAG, "Unknown setting type!");
    return "";
}

std::string Settings::defaultSetting(Settings::String settingType)
{
    switch(settingType)
    {
        case Settings::String::SERIAL_ID: return "UNDEFINED";
        case Settings::String::PLAYLIST: return "playlist.pls";
        case Settings::String::PRINT_GALLERY: return "";
        case Settings::String::WIFI_SSID: return "Kinetic_table";
        case Settings::String::WIFI_PASSWORD: return "1234567890";
        case Settings::String::LED_TYPE: return "PWM";
    }
    ESP_LOGE(TAG, "Unknown setting type!");
    return "";
}