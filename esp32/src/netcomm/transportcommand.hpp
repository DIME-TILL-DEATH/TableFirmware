#ifndef TRANSPORTCOMMAND_H
#define TRANSPORTCOMMAND_H

#include "abstractcommand.hpp"
namespace NetComm
{


class TransportCommand : public AbstractCommand
{
public:
    typedef enum 
    {
        PREVIOUS_PRINT,
        NEXT_PRINT,
        PAUSE_PRINTING,
        SET_PRINT,
        REQUEST_PROGRESS
    }TransportActions;

    struct 
    {
        uint16_t currentPoint;
        uint16_t printPoints;
    }progress;

    TransportCommand(uint8_t commId, TransportActions action, Direction dir = Direction::REQUEST);

    TransportActions action() {return m_action;};
private:
    TransportActions m_action;
};

};
#endif