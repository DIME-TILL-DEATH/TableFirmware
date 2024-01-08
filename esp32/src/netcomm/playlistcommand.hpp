#ifndef PLAYLISTCOMMAND_H
#define PLAYLISTCOMMAND_H

#include <string>
#include <vector>

#include "abstractcommand.hpp"
namespace NetComm
{


class PlaylistCommand : public AbstractCommand
{
public:
    typedef enum 
    {
        REQUEST_PLAYLIST,
        CHANGE_PLAYLIST
    }PlaylistActions;

    struct 
    {
        uint16_t currentPoint;
        uint16_t printPoints;
    }progress;

    PlaylistCommand(uint8_t commId, PlaylistActions action, Direction dir = Direction::REQUEST);

    PlaylistActions action() {return m_action;};

    std::vector<std::string>* playlist_ptr;
    int16_t curPlsPos{-1};
private:
    PlaylistActions m_action;
};

};

#endif