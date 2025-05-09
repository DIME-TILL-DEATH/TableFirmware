#ifndef SETTINGS_H
#define SETTINGS_H

#include <math.h>

#include <stdint.h>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class Settings{
public:

    enum class String
    {
        SERIAL_ID,
        WIFI_SSID,
        WIFI_PASSWORD,
        PLAYLIST,
        PRINT_GALLERY,
        LED_TYPE
    };

    enum class Digit
    {
        PRINT_SPEED,
        SCALE_COEF,
        PRINT_ROTATION,
        LAST_PLAYLIST_POSITION,
        LED_BRIGHTNESS,
        CORRETION_LENGTH,
        PAUSE_INTERVAL,
        FI_GEAR2_TEETH_COUNT,
        MACHINE_MINUTES,
        FIRST_MOTOR_INVERSION,
        SECOND_MOTOR_INVERSION
    };

    static void checkSettingsFile();

    static float_t getSetting(Settings::Digit setting);
    static std::string getSetting(Settings::String setting);

    static void saveSetting(Settings::Digit setting, float_t value);
    static void saveSetting(Settings::String setting, std::string value);
private:
    static std::string settingName(Settings::Digit settingType);
    static std::string settingName(Settings::String settingType);

    static float_t defaultSetting(Settings::Digit setting);
    static std::string defaultSetting(Settings::String setting);

    constexpr static char TAG[] = "SETTINGS";

    SemaphoreHandle_t m_spiMutex;
};

extern SemaphoreHandle_t spiMutex;

#endif