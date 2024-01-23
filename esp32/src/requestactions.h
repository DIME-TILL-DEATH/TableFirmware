#ifndef REQUESTACTIONS_H
#define REQUESTACTIONS_H

namespace Requests
{
enum class Transport
{
    PAUSE_PRINTING,
    SET_PRINT,
    REQUEST_PROGRESS
};

enum class Playlist
{
    REQUEST_PLAYLIST,
    REQUEST_PLAYLIST_POSITION,
    CHANGE_PLAYLIST,
    CHANGE_PLAYLIST_POSITION
};

enum class File
{
    GET_FILE,
    GET_FOLDER_CONTENT,
    FILE_CREATE,
    FILE_APPEND_DATA
};

}

namespace Data
{
enum class Transport
{
    PROGRESS,
    PAUSE_STATE
};

enum class Playlist
{
    PLAYLIST,
    PLAYLIST_POSITION
};

enum class File
{
    REQUESTED_FILE,
    REQUESTED_FOLDER
};

}

#endif // REQUESTACTIONS_H
