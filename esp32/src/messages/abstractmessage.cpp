#include "abstractmessage.h"

AbstractMessage::AbstractMessage(FrameType frameType, uint8_t action)
{
    memset(m_frameHeader.rawData, 0, sizeof(FrameHeader));

    m_frameHeader.structData.frameType = frameType;
    m_frameHeader.structData.action = action;
    m_frameHeader.structData.frameSize = sizeof(FrameHeader);
}

AbstractMessage::AbstractMessage(QByteArray rawData)
    : m_rawData(rawData)
{
    if(m_rawData.size() >= sizeof(FrameHeader))
    {
        memcpy(m_frameHeader.rawData, m_rawData.data(), sizeof(FrameHeader));
    }
}

AbstractMessage::~AbstractMessage()
{
}

QByteArray AbstractMessage::rawData() const
{
    return m_rawData;
}
