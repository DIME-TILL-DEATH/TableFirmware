#ifndef TRANSPORTCOMMAND_H
#define TRANSPORTCOMMAND_H

#include "requestactions.h"
#include "abstractcommand.hpp"

namespace NetComm
{

class TransportCommand : public AbstractCommand
{
public:
    struct 
    {
        uint16_t currentPoint;
        uint16_t printPoints;
    }progress;

    TransportCommand(uint8_t commId, Requests::Transport action, Direction dir = Direction::REQUEST)
            : AbstractCommand(commId, dir)
    {
        m_commandType = CommandType::TRANSPORT_COMMAND;
        m_action = action;
    }

    Requests::Transport action() {return m_action;};
private:
    Requests::Transport m_action;
};

};
#endif