#include "transportcommand.hpp"

using namespace NetComm;

TransportCommand::TransportCommand(uint8_t commId, TransportActions action, Direction dir)
    : AbstractCommand(commId, dir)
{
    m_commandType = CommandType::TRANSPORT_COMMAND;
    m_action = action;
}