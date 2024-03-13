#ifndef SETTINGS_H
#define SETTINGS_H

#include <math.h>

#include <stdint.h>
#include <string>

class Settings{
public:

    enum class String
    {
        WIFI_SSID,
        WIFI_PASSWORD,
        PLAYLIST
    };

    enum class Digit
    {
        PRINT_SPEED,
        SCALE_COEF,
        PRINT_ROTATION,
        LAST_PLAYLIST_POSITION,
        LED_BRIGHTNESS,
        CORRETION_LENGTH,
        PAUSE_INTERVAL
    };

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
};

#endif