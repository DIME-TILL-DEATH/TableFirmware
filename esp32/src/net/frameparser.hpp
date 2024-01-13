#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <vector>

#include "frames.h"

#include "netcomm/abstractcommand.hpp"
#include "netcomm/transportcommand.hpp"
#include "netcomm/playlistcommand.hpp"
#include "netcomm/filecommand.hpp"

class FrameParser
{
public:
    FrameParser(int socket);
    void processRecvData(uint8_t* frame, uint16_t len);

    std::vector<NetComm::AbstractCommand*> parsedCommands;
private:
    int m_socket;

    uint32_t curFrameBytesRecv{0}
    std::vector<uint8_t> txBuffer;
    std::vector<uint8_t> lastRecvFrame;
    FrameHeader lastRecvFrameHeader;

    void parseTransportActions();
    void parsePlaylistActions();
    void parseFileActions();
};

#endif