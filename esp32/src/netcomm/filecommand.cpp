#include "filecommand.hpp"

using namespace NetComm;

FileCommand::FileCommand(uint8_t commId, Requests::File action, Direction dir)
    : AbstractCommand(commId, dir)
{
    m_commandType = CommandType::FILE_COMMAND;
    m_action = action;
}