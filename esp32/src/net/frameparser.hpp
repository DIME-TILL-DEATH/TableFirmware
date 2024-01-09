#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <vector>

#include "frames.h"

#include "netcomm/abstractcommand.hpp"
#include "netcomm/transportcommand.hpp"
#include "netcomm/playlistcommand.hpp"

class FrameParser
{
public:
    FrameParser(int socket);
    void processRecvData(uint8_t* frame, uint16_t len);

    NetComm::AbstractCommand* lastRecvCommand() {return m_command;};
    std::vector<NetComm::AbstractCommand*> parsedCommands;
private:
    int m_socket;
    // uint8_t m_frameBuffer[2048];
    // uint16_t m_bufferPos{0};
    std::vector<uint8_t> recvData;
    FrameHeader lastRecvFrameHeader;

    NetComm::AbstractCommand* m_command{nullptr};
};

#endif