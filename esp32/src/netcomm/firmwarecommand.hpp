#ifndef FIRMWARECOMMAND_H
#define FIRMWARECOMMAND_H

#include <string>

#include "abstractcommand.hpp"
#include "requestactions.h"

namespace NetComm
{

class FirmwareCommand : public AbstractCommand
{
public:
    struct 
    {
        uint16_t currentPoint;
        uint16_t printPoints;
    }progress;

    FirmwareCommand(uint8_t commId, Requests::Firmware action, Direction dir = Direction::REQUEST)
                    : AbstractCommand(commId, dir)
    {
        m_commandType = CommandType::FIRMWARE_COMMAND;
        m_action = action;
    }

    Requests::Firmware action() {return m_action;};

    std::string firmwareVersion;
    int32_t dataProcessed{0};
    int32_t fileSize{0};
private:
    Requests::Firmware m_action;
};

};

#endif