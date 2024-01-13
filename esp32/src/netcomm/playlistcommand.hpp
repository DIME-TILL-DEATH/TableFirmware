#ifndef PLAYLISTCOMMAND_H
#define PLAYLISTCOMMAND_H

#include <string>
#include <vector>

#include "abstractcommand.hpp"
#include "requestactions.h"

namespace NetComm
{


class PlaylistCommand : public AbstractCommand
{
public:
    struct 
    {
        uint16_t currentPoint;
        uint16_t printPoints;
    }progress;

    PlaylistCommand(uint8_t commId, Requests::Playlist action, Direction dir = Direction::REQUEST);

    Requests::Playlist action() {return m_action;};

    std::vector<std::string>* playlist_ptr;
    int16_t curPlsPos;
private:
    Requests::Playlist m_action;
};

};

#endif