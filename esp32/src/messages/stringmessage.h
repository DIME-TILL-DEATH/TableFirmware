#ifndef STRINGMESSAGE_H
#define STRINGMESSAGE_H


#include "abstractmessage.h"

#include "utils/qt_compat.h"

class StringMessage : public AbstractMessage
{
public:
    StringMessage(FrameType frameType, uint8_t action, const QString& string = {});
    StringMessage(const QByteArray& recievedData);
    ~StringMessage();

    QString string() const {return m_string;};

private:
    QString m_string;
};

#endif // STRINGMESSAGE_H
