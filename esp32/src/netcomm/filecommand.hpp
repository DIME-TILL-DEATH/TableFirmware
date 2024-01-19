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
    FileCommand(uint8_t commId, Requests::File action, Direction dir = Direction::REQUEST);

    std::string path;

    Requests::File action() {return m_action;};
private:
    Requests::File m_action;
};

};

#endif