#ifndef HARDWARECOMMAND_H
#define HARDWARECOMMAND_H

#include <math.h>

#include "requestactions.h"
#include "abstractcommand.hpp"

#include "net/frames.h"

namespace NetComm
{

class HardwareCommand : public AbstractCommand
{
public:
    HardwareCommand(uint8_t commId, Requests::Hardware action, Direction dir = Direction::REQUEST);
    HardwareCommand(uint8_t commId, const FrameHeader& frameHeader, Direction dir = Direction::REQUEST);

    void formAnswerFrame(uint8_t* data, uint32_t* size);

    // all to pointer on data?
    struct 
    {
        uint16_t currentPoint;
        uint16_t printPoints;
    }progress;

    uint32_t pauseInterval;
    float_t printSpeed;
    float_t ledBrightness;
    float_t scaleCoefficient;
    float_t rotation;
    float_t correction;
    
    uint16_t fiGear2Teeths;

    // TODO: new syntaxis
    void* dataPtr{NULL};
    uint32_t dataSize{0};

    Requests::Hardware action() {return m_action;};
private:
    Requests::Hardware m_action;
};

};
#endif