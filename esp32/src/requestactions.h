#ifndef REQUESTACTIONS_H
#define REQUESTACTIONS_H

#include <stdint.h>

typedef enum : uint8_t
{
    UNDEFINED = 0,
    PLAYLIST_ACTIONS,
    HARDWARE_ACTIONS,
    FILE_ACTIONS,
    FIRMWARE_ACTIONS
}FrameType;

namespace Requests
{
enum class Hardware
{
    PAUSE_PRINTING,
    REQUEST_PROGRESS,
    GET_PRINT_SPEED,
    SET_PRINT_SPEED,
    GET_LED_BRIGHTNESS,
    SET_LED_BRIGHTNESS,
    GET_SCALE_COEFFICIENT,
    SET_SCALE_COEFFICIENT,
    GET_ROTATION,
    SET_ROTATION,
    GET_CORRECTION,
    SET_CORRECTION,
    GET_PAUSE_INTERVAL,
    SET_PAUSE_INTERVAL
};

enum class Playlist
{
    REQUEST_PLAYLIST,
    REQUEST_PLAYLIST_POSITION,
    CHANGE_PLAYLIST,
    CHANGE_PLAYLIST_POSITION,
    CHANGE_PRINTNG_FILE
};

enum class File
{
    GET_FILE,
    GET_FOLDER_CONTENT,
    FILE_CREATE,
    FILE_APPEND_DATA
};

enum class Firmware
{
    FIRMWARE_VERSION,
    FIRMWARE_UPLOAD_START,
    FIRMWARE_UPLOAD_PROCEED,
    FIRMWARE_UPDATE,
    ESP_RESTART
};

}

// namespace Data
// {
// enum class Hardware
// {
//     PROGRESS,
//     PAUSE_STATE
// };

// enum class Playlist
// {
//     PLAYLIST,
//     PLAYLIST_POSITION
// };

// enum class File
// {
//     REQUESTED_FILE,
//     REQUESTED_FOLDER
// };
// }

#endif // REQUESTACTIONS_H
