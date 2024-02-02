#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"

#include "settings.hpp"
#include "filemanager.hpp"

float_t Settings::getDigitSetting(Settings::Digit setting)
{
    float_t settingValue=defaultDigitSetting(setting);;
    char buf[512];

    std::string settingRequest = digitSettingName(setting) + "=" + "%f";

    std::string fullFileName;
    fullFileName = FileManager::mountPoint + "settings.ini";
    FILE* file = fopen(fullFileName.c_str(), "r");
    if(file == NULL)
    {
        ESP_LOGE(TAG, "Can't open settings file");
        goto ENDING;
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
    fclose(file);
    return settingValue;
}
std::string Settings::getStringSetting(Settings::String setting)
{
    std::string settingValue=defaultStringSetting(setting);;
    char buf[512];

    std::string settingRequest = stringSettingName(setting) + "=" + "%s";

    std::string fullFileName;
    fullFileName = FileManager::mountPoint + "settings.ini";
    FILE* file = fopen(fullFileName.c_str(), "r");
    if(file == NULL)
    {
        ESP_LOGE(TAG, "Can't open settings file");
        goto ENDING;
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
    fclose(file);
    return settingValue;
}

std::string Settings::digitSettingName(Settings::Digit settingType)
{
    switch(settingType)
    {
        case Settings::Digit::PRINT_ROTATION: return "PRINT_ROTATION";
        case Settings::Digit::PRINT_SPEED: return "PRINT_SPEED";
        case Settings::Digit::SCALE_COEF: return "SCALE_COEF";
        case Settings::Digit::LAST_PLAYLIST_POSITION: return "LAST_PLAYLIST_POSITION";
    }
    ESP_LOGE(TAG, "Unknown setting type!");
    return "";
}

float_t Settings::defaultDigitSetting(Settings::Digit settingType)
{
    switch(settingType)
    {
        case Settings::Digit::PRINT_ROTATION: return 0;
        case Settings::Digit::PRINT_SPEED: return 25;
        case Settings::Digit::SCALE_COEF: return 1.0;
        case Settings::Digit::LAST_PLAYLIST_POSITION: return 0;
    }
    ESP_LOGE(TAG, "Unknown setting type!");
    return 0;
}

std::string Settings::stringSettingName(Settings::String settingType)
{
    switch(settingType)
    {
        case Settings::String::PLAYLIST: return "PLAYLIST";
        case Settings::String::WIFI_SSID: return "WIFI_SSID";
        case Settings::String::WIFI_PASSWORD: return "WIFI_PASSWORD";
    }
    ESP_LOGE(TAG, "Unknown setting type!");
    return "";
}

std::string Settings::defaultStringSetting(Settings::String settingType)
{
    switch(settingType)
    {
        case Settings::String::PLAYLIST: return "playlist.pls";
        case Settings::String::WIFI_SSID: return "Kinetic_table";
        case Settings::String::WIFI_PASSWORD: return "1234567890";
    }
    ESP_LOGE(TAG, "Unknown setting type!");
    return "";
}