#ifndef NETCOMM_ABSTRACTCOMMAND_H
#define NETCOMM_ABSTRACTCOMMAND_H

#include <stdint.h>

namespace NetComm
{

typedef enum 
{
    ABSTRACT = 0,
    HARDWARE_COMMAND,
    PLAYLIST_COMMAND,
    FILE_COMMAND,
    FIRMWARE_COMMAND
}CommandType;

typedef enum
{
    REQUEST = 1,
    ANSWER
}Direction;

class AbstractCommand
{
public:
    AbstractCommand(uint8_t commId, Direction setDirection = Direction::REQUEST) 
    {
        m_commId = commId;
        direction = setDirection;
    }

    uint8_t commId() {return m_commId;}
    CommandType commandType() {return m_commandType;}
    Direction direction;
protected:
    CommandType m_commandType{CommandType::ABSTRACT};
    uint8_t m_commId{0};
   // Direction m_direction;
};

};
#endif