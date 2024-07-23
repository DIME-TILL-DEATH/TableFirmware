#ifndef INTVALUEMESSAGE_H
#define INTVALUEMESSAGE_H

#include "abstractmessage.h"

class IntValueMessage : public AbstractMessage
{
public:
    enum class ActionType
    {
        NOT_INT_VALUE,
        SET,
        GET
    };

    IntValueMessage(FrameType frameType, uint8_t action, uint32_t value1 = 0, uint32_t value2 = 0);
    IntValueMessage(QByteArray recievedData);
    ~IntValueMessage();

    ActionType actionType() const {return m_actionType;};

    uint32_t value() const {return m_value1;};
    uint32_t firstValue() const {return m_value1;};
    uint32_t secondValue() const {return m_value2;};

private:
    uint32_t m_value1;
    uint32_t m_value2;

    ActionType m_actionType;

    ActionType recognizeActionType();
};

#endif // INTVALUEMESSAGE_H
