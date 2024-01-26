#ifndef NETCOMM_ABSTRACTCOMMAND_H
#define NETCOMM_ABSTRACTCOMMAND_H

#include <stdint.h>

namespace NetComm
{

typedef enum 
{
    ABSTRACT = 0,
    TRANSPORT_COMMAND,
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
    AbstractCommand(uint8_t commId, Direction direction = Direction::REQUEST) 
    {
        m_commId = commId;
        m_direction = direction;
    }

    uint8_t commId() {return m_commId;}
    CommandType commandType() {return m_commandType;}
    Direction direction() {return m_direction;}
protected:
    CommandType m_commandType{CommandType::ABSTRACT};
    uint8_t m_commId{0};
    Direction m_direction;
};

};
#endif