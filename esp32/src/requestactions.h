#ifndef REQUESTACTIONS_H
#define REQUESTACTIONS_H

namespace Requests
{
enum class Transport
{
    PREVIOUS_PRINT=0,
    NEXT_PRINT,
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
    GET_FILE
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
    REQUESTED_FILE
};

}

#endif // REQUESTACTIONS_H
