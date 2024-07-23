#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <vector>
#include <queue>

#include "utils\qt_compat.h"

#include "frames.h"

#include "messages/abstractmessage.h"
#include "messages/intvaluemessage.h"
#include "messages/floatvaluemessage.h"
#include "messages/stringmessage.h"
#include "messages/filepartmessage.h"
#include "messages/foldercontentmessage.h"

class FrameParser
{
public:
    FrameParser(int socket);
    ~FrameParser();

    void processRecvData(uint8_t* frame, uint16_t len);

    std::queue<AbstractMessage*> parsedMessages;
private:
    int m_socket;

    int32_t curFrameBytesRecv{0};
    QByteArray txBuffer;
    QByteArray lastRecvFrame;
    FrameHeader lastRecvFrameHeader;

    AbstractMessage* parseHardwareActions();
    AbstractMessage* parsePlaylistActions();
    AbstractMessage* parseFileActions();
    AbstractMessage* parseFirmwareActions();
};

#endif