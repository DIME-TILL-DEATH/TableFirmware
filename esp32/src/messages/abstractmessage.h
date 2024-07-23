#ifndef ABSTRACTMESSAGE_H
#define ABSTRACTMESSAGE_H

#include "utils\qt_compat.h"

class AbstractMessage
{
public:
    typedef enum
    {
        ABSTRACT,
        INT_VALUE,
        FLOAT_VALUE,
        STRING,
        FILE_PART,
        FOLDER_CONTENT
    }MessageType;

    AbstractMessage(FrameType frameType, uint8_t action);
    AbstractMessage(QByteArray rawData);
    virtual ~AbstractMessage();

    QByteArray rawData() const;

    MessageType messageType() const {return m_messageType;};
    FrameType frameType() const {return m_frameHeader.structData.frameType;};
    uint8_t action() const {return m_frameHeader.structData.action;};

protected:
    MessageType m_messageType{MessageType::ABSTRACT};
    FrameHeader_uni m_frameHeader;

    QByteArray m_rawData;
};

#endif // ABSTRACTMESSAGE_H
