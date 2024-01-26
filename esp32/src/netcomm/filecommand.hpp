#ifndef FILECOMMAND_H
#define FILECOMMAND_H

#include "requestactions.h"
#include "abstractcommand.hpp"

#include <string>

namespace NetComm
{

class FileCommand : public AbstractCommand
{
public:
    FileCommand(uint8_t commId, Requests::File action, Direction dir = Direction::REQUEST)
        : AbstractCommand(commId, dir)
    {
        m_commandType = CommandType::FILE_COMMAND;
        m_action = action;
    }

    std::string path;
    int32_t dataProcessed{0};

    Requests::File action() {return m_action;};
private:
    Requests::File m_action;
};

};

#endif