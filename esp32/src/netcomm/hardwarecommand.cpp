#include "hardwarecommand.hpp"

#include <stdio.h>
#include <string.h>

using namespace NetComm;

HardwareCommand::HardwareCommand(uint8_t commId, Requests::Hardware action, Direction dir)
            : AbstractCommand(commId, dir)
{
    m_commandType = CommandType::HARDWARE_COMMAND;
    m_action = action;
}

HardwareCommand::HardwareCommand(uint8_t commId, const FrameHeader& frameHeader, Direction dir)
            : AbstractCommand(commId, dir)
{
    m_commandType = CommandType::HARDWARE_COMMAND;
    m_action = (Requests::Hardware)frameHeader.action;

    union { float f; uint32_t i; } u;
    u.i = frameHeader.data0;

    switch((Requests::Hardware)frameHeader.action)
    {
        case Requests::Hardware::SET_PRINT_SPEED:
        {
            printSpeed = u.f;
            break;
        }

        case Requests::Hardware::SET_LED_BRIGHTNESS:
        {
            ledBrightness = u.f;
            break;
        }

        case Requests::Hardware::SET_SCALE_COEFFICIENT:
        {
            scaleCoefficient = u.f;
            break;
        }

        case Requests::Hardware::SET_ROTATION:
        {
            rotation = u.f;
            break;
        }

        case Requests::Hardware::SET_CORRECTION:
        {
            correction = u.f;
            break;
        }

        case Requests::Hardware::SET_PAUSE_INTERVAL:
        {
            pauseInterval = frameHeader.data0;
        }
        default: {};
    }
}

void HardwareCommand::formAnswerFrame(uint8_t* data, uint32_t* size)
{
    FrameHeader_uni answerFrame;
    memset(answerFrame.rawData, 0, sizeof(FrameHeader));
    answerFrame.structData.frameType = FrameType::HARDWARE_ACTIONS;
    answerFrame.structData.action = (uint8_t)m_action;
    answerFrame.structData.frameSize = sizeof(FrameHeader);

    switch(m_action)
    {
        case Requests::Hardware::PAUSE_PRINTING:
        {
            
            break;
        }

        case Requests::Hardware::REQUEST_PROGRESS:
        {
            answerFrame.structData.data0 = progress.currentPoint;    
            answerFrame.structData.data1 = progress.printPoints; 
            break;
        }

        case Requests::Hardware::SET_PRINT_SPEED:
        {
            answerFrame.structData.action = (uint8_t)Requests::Hardware::GET_PRINT_SPEED;
        }

        case Requests::Hardware::GET_PRINT_SPEED:
        {
            union { float f; uint32_t i; } u;
            u.f = printSpeed;
            answerFrame.structData.data0 = u.i;
            break;
        }

        case Requests::Hardware::SET_LED_BRIGHTNESS:
        {
            answerFrame.structData.action = (uint8_t)Requests::Hardware::GET_LED_BRIGHTNESS;
        }

        case Requests::Hardware::GET_LED_BRIGHTNESS:
        {
            union { float f; uint32_t i; } u;
            u.f = ledBrightness;
            answerFrame.structData.data0 = u.i;    
            break;
        }

        case Requests::Hardware::SET_SCALE_COEFFICIENT:
        {
            answerFrame.structData.action = (uint8_t)Requests::Hardware::GET_SCALE_COEFFICIENT;
        }

        case Requests::Hardware::GET_SCALE_COEFFICIENT:
        {
            union { float f; uint32_t i; } u;
            u.f = scaleCoefficient;
            answerFrame.structData.data0 = u.i;       
            break;
        }

        case Requests::Hardware::SET_ROTATION:
        {
            answerFrame.structData.action = (uint8_t)Requests::Hardware::GET_ROTATION;
        }

        case Requests::Hardware::GET_ROTATION:
        {
            union { float f; uint32_t i; } u;
            u.f = rotation;
            answerFrame.structData.data0 = u.i;       
            break;
        }

        case Requests::Hardware::SET_CORRECTION:
        {
            answerFrame.structData.action = (uint8_t)Requests::Hardware::GET_CORRECTION;
        }

        case Requests::Hardware::GET_CORRECTION:
        {
            union { float f; uint32_t i; } u;
            u.f = correction;
            answerFrame.structData.data0 = u.i;       
            break;
        }

         case Requests::Hardware::SET_PAUSE_INTERVAL:
        {
            answerFrame.structData.action = (uint8_t)Requests::Hardware::GET_PAUSE_INTERVAL;
        }

        case Requests::Hardware::GET_PAUSE_INTERVAL:
        {
            answerFrame.structData.data0 = pauseInterval;    
            break;
        }
    }

    memcpy(data, answerFrame.rawData, sizeof(FrameHeader)) ;
    *size = sizeof(FrameHeader);
}