#include "stringmessage.h"

StringMessage::StringMessage(FrameType frameType, uint8_t action, const QString& string)
    : AbstractMessage(frameType, action),
    m_string(string)
{
    m_messageType = AbstractMessage::STRING;

    m_frameHeader.structData.frameSize = sizeof(FrameHeader) + string.size();
    m_frameHeader.structData.data0 = 0;
    m_frameHeader.structData.data1 = 0;
    m_frameHeader.structData.frameParameters = m_string.size();

    m_rawData.append(m_frameHeader.rawData, sizeof(FrameHeader));
    // m_rawData.append(m_string.toLocal8Bit());
    m_rawData.append(m_string.c_str(), m_string.size());

}

StringMessage::StringMessage(const QByteArray& recievedData)
    : AbstractMessage(recievedData)
{
    m_messageType = AbstractMessage::STRING;

    m_string = m_rawData.right(m_rawData.size() - sizeof(FrameHeader));
} 

StringMessage::~StringMessage()
{

}
