#include "floatvaluemessage.h"

FloatValueMessage::FloatValueMessage(FrameType frameType, uint8_t action, float value)
    : AbstractMessage(frameType, action),
    m_value(value)
{
    m_messageType = AbstractMessage::FLOAT_VALUE;

    union { float f; uint32_t i; } u;
    u.f = m_value;

    m_frameHeader.structData.data0 = u.i;
    m_frameHeader.structData.data1 = 0;
    m_frameHeader.structData.frameParameters = 0;

    m_rawData.append(m_frameHeader.rawData, sizeof(FrameHeader));
}

FloatValueMessage::FloatValueMessage(QByteArray recievedData)
    : AbstractMessage(recievedData)
{
    m_messageType = AbstractMessage::FLOAT_VALUE;

    union { float f; uint32_t i; } u;
    u.i = m_frameHeader.structData.data0;

    m_value = u.f;

    m_actionType = recognizeActionType();
}

FloatValueMessage::~FloatValueMessage()
{

}

FloatValueMessage::ActionType FloatValueMessage::recognizeActionType()
{
    switch (m_frameHeader.structData.frameType)
    {
    case HARDWARE_ACTIONS:
    {
        switch(static_cast<Requests::Hardware>(m_frameHeader.structData.action))
        {
        case Requests::Hardware::GET_PRINT_SPEED: return ActionType::GET;
        case Requests::Hardware::SET_PRINT_SPEED: return ActionType::SET;
        case Requests::Hardware::GET_LED_BRIGHTNESS: return ActionType::GET;
        case Requests::Hardware::SET_LED_BRIGHTNESS: return ActionType::SET;
        case Requests::Hardware::GET_SCALE_COEFFICIENT: return ActionType::GET;
        case Requests::Hardware::SET_SCALE_COEFFICIENT: return ActionType::SET;
        case Requests::Hardware::GET_ROTATION: return ActionType::GET;
        case Requests::Hardware::SET_ROTATION: return ActionType::SET;
        case Requests::Hardware::GET_CORRECTION: return ActionType::GET;
        case Requests::Hardware::SET_CORRECTION: return ActionType::SET;
        default: return ActionType::NOT_FLOAT_VALUE;
        }
    }

    default: return ActionType::NOT_FLOAT_VALUE;
    }
}
