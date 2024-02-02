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
        LAST_PLAYLIST_POSITION
    };

    static float_t getDigitSetting(Settings::Digit setting);
    static std::string getStringSetting(Settings::String setting);
private:
    static std::string digitSettingName(Settings::Digit settingType);
    static std::string stringSettingName(Settings::String settingType);

    static float_t defaultDigitSetting(Settings::Digit setting);
    static std::string defaultStringSetting(Settings::String setting);

    constexpr static char TAG[] = "SETTINGS";
};

#endif