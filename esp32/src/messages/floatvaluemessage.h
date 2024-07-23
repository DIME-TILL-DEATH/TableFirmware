#ifndef FLOATVALUEMESSAGE_H
#define FLOATVALUEMESSAGE_H

#include "abstractmessage.h"

class FloatValueMessage : public AbstractMessage
{
public:
    enum class ActionType
    {
        NOT_FLOAT_VALUE,
        SET,
        GET
    };

    FloatValueMessage(FrameType frameType, uint8_t action, float value = 0);
    FloatValueMessage(QByteArray recievedData);
    ~FloatValueMessage();

    ActionType actionType() const {return m_actionType;};
    float value() const {return m_value;};

private:
    ActionType m_actionType;
    ActionType recognizeActionType();

    float m_value;
};

#endif // FLOATVALUEMESSAGE_H
