#include "playlistcommand.hpp"

using namespace NetComm;

PlaylistCommand::PlaylistCommand(uint8_t commId, PlaylistActions action, Direction dir)
    : AbstractCommand(commId, dir)
{
    m_commandType = CommandType::PLAYLIST_COMMAND;
    m_action = action;
}