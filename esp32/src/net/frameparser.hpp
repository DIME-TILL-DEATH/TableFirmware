#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include "frames.h"

#include "netcomm/abstractcommand.hpp"
#include "netcomm/transportcommand.hpp"

class FrameParser
{
public:
    FrameParser(int socket);
    void processRecvData(uint8_t* frame, uint16_t len);

    NetComm::AbstractCommand* lastRecvCommand() {return m_command;};
private:
    int m_socket;
    uint8_t m_frameBuffer[2048];

    NetComm::AbstractCommand* m_command{nullptr};
};

#endif