#include <math.h>

#include "esp_log.h"
#include "printer.hpp"

Printer::Printer()
{
    m_state = PrinterState::IDLE;

    currentPosition.x = 0;
    currentPosition.y = 0;

    currentPolarPosition.r = 0;
    currentPolarPosition.fi = 0;
}

void Printer::initTimers(gptimer_alarm_cb_t rTimerCb,
                    gptimer_alarm_cb_t fiTimerCb)
{  
    rTimer = new IntervalTimer(rTimerCb);
    fiTimer = new IntervalTimer(fiTimerCb);
}

void Printer::initPins()
{
    printerPins = new Pins::PrinterPins();


    // EXTI_InitTypeDef EXTI_InitStructure = {0};

    // pinRSensor.pin = GPIO_Pin_10;
    // pinRSensor.port = GPIOB;
    // GPIO_InitStructure.GPIO_Pin = pinRSensor.pin;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // GPIO_Init(pinRSensor.port, &GPIO_InitStructure);

    // GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);
    // EXTI_InitStructure.EXTI_Line = EXTI_RSENS_LINE;
    // EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    // EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    // EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    // EXTI_Init(&EXTI_InitStructure);

    // pinFiSensor.pin = GPIO_Pin_13;
    // pinFiSensor.port = GPIOB;
    // GPIO_InitStructure.GPIO_Pin = pinRSensor.pin;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // GPIO_Init(pinRSensor.port, &GPIO_InitStructure);

    // GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
    // EXTI_InitStructure.EXTI_Line = EXTI_FISENS_LINE;
    // EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    // EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    // EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    // EXTI_Init(&EXTI_InitStructure);
}

void Printer::findCenter()
{
    pauseThread();

    rTimer->setInterval(350);
    fiTimer->setInterval(350);

    printf("=>Finding table center...\r\n");
    currentPosition.x = 0;
    currentPosition.y = 0;

    pointNum = 1;

    fiCenterTrigger = false;
    rCenterTrigger = false;

    printerPins->rDirState(Pins::PinState::RESET);
    printerPins->fiDirState(Pins::PinState::SET);

    if(true) //GPIO_ReadInputDataBit(pinFiSensor.port, pinFiSensor.pin) == Bit_SET)
    {
        fiCenterTrigger = true;
    }
    else
    {
        setStep(0, -2*M_PI, 30);
    }

    if(true)//GPIO_ReadInputDataBit(pinRSensor.port, pinRSensor.pin) == Bit_RESET)
    {
        rCenterTrigger = true;
    }

    // EXTI_ClearITPendingBit(EXTI_FISENS_LINE | EXTI_RSENS_LINE);
    // NVIC_EnableIRQ(EXTI15_10_IRQn);

    m_state = PrinterState::SEARCH_FI_ZERO;

    resumeThread();
}

void Printer::pushPrintCommand(GCode::GAbstractComm* command)
{
    m_printJob.push(command);
}

void Printer::printRoutine()
{
    switch(m_state)
    {
        case PrinterState::IDLE:
        {
            if(m_printJob.size()>0)
            {
                m_state = PrinterState::HANDLE_COMMAND;
            }
            break;
        }

        case PrinterState::HANDLE_COMMAND:
        {
            if(m_printJob.size()>0)
            {
                GCode::GAbstractComm* aComm = m_printJob.front();
                m_printJob.pop();

                switch(aComm->commType())
                {
                    case GCode::GCommType::M51:
                    {
                        findCenter();
                        break;
                    }
                    case GCode::GCommType::G1:
                    {
                        GCode::G1Comm* g1Comm = static_cast<GCode::G1Comm*>(aComm);
                        if(g1Comm)
                        {
                            targetPosition = g1Comm->decartCoordinates();
                            targetPosition.x = targetPosition.x * printScaleCoef;
                            targetPosition.y = targetPosition.y * printScaleCoef;

                            m_state = PrinterState::SET_POINT;

                            printf("point(%lf, %lf)", targetPosition.x, targetPosition.y);
                        }
                        break;
                    }
                    case GCode::GCommType::G4:
                    {
                        GCode::G4Comm* g4Comm = static_cast<GCode::G4Comm*>(aComm);
                        if(g4Comm)
                        {
                            printf("End of file.\r\n");
                            pauseThread();
                           // Delay_Ms(g4Comm->pause()); //  ?????m_state = PrinterState::PAUSE;
                            resumeThread();
                        }
                        break;
                    }
                    default: ESP_LOGE("PRINTER", "Unhandled CGomm");
                }
                delete aComm;
            }
            else
            {
                m_state = PrinterState::IDLE;
            }
            break;
        }

        case PrinterState::SET_POINT:
        {
            if(targetPosition.x == currentPosition.x && currentPosition.y == targetPosition.y)
            {
                m_state = PrinterState::HANDLE_COMMAND;
                return; //skip point
            }

            float_t deltaX = targetPosition.x - currentPosition.x;
            float_t deltaY = targetPosition.y - currentPosition.y;
            targetPolarPosition = Coord::convertDecartToPolar(targetPosition);

            float_t lineLength = sqrt(pow((deltaX), 2) + pow((deltaY), 2));
            float_t steps = lineLength/stepSize;
            stepX = deltaX / steps;
            stepY = deltaY / steps;

            stepTime =  (sqrt(pow((stepX), 2) + pow((stepY), 2))) / speed;

            // INFO block==========
                printf(", point num: %d\r\n", pointNum);
//                printf("DECART: current(%lf, %lf), target(%lf, %lf)\r\n", currentPosition.x, currentPosition.y, targetPosition.x, targetPosition.y);
//                printf("length: %lf, steps: %lf, stepX: %lf, stepY: %lf\r\n", lineLength, steps, stepX, stepY);
//                printf("POLAR: current(%lf, %lf), target(%lf, %lf)\r\n", currentPolarPosition.r, currentPolarPosition.fi* 360 / (M_PI * 2), targetPolarPosition.r, targetPolarPosition.fi* 360 / (M_PI * 2));
//                printf("\r\n");
            //=================================

            pointNum++;

            m_state = PrinterState::SET_STEP;
            break;
        }

        case PrinterState::SET_STEP:
        {
            double_t minErrX = (stepX == 0) ? stepSize : fabs(stepX);
            double_t minErrY = (stepY == 0) ? stepSize : fabs(stepY);

            if((fabs(targetPosition.x - currentPosition.x) < minErrX) && (fabs(targetPosition.y - currentPosition.y) < minErrY))
            {
                m_state = PrinterState::HANDLE_COMMAND;
            }
            else
            {
                Coord::DecartPoint stepPosition{currentPosition.x + stepX, currentPosition.y +stepY};

                Coord::PolarPoint curPolarPoint = Coord::convertDecartToPolar(currentPosition);
                Coord::PolarPoint stepPolarPoint = Coord::convertDecartToPolar(stepPosition);

                double_t deltaR = stepPolarPoint.r - curPolarPoint.r;
                double_t deltaFi = stepPolarPoint.fi - curPolarPoint.fi;

                // x-axis positive cross(polar +2pi) not align
                if(fabs(deltaFi) > M_PI)
                {
                    if(Coord::isLinesCross(currentPosition, stepPosition, {0, 0}, {1000, 0}))
                    {
                        deltaFi = (-1) * copysign(2 * M_PI - fabs(deltaFi), deltaFi);
                    }
                }

                setStep(deltaR, deltaFi, stepTime);

                currentPosition.x += stepX;
                currentPosition.y += stepY;

                // Avoid overflow on circular printing in one direction
                if(currentPolarPosition.fi < 0)
                {
                    currentPolarPosition.fi += 2 * M_PI;
                }
                if(currentPolarPosition.fi > 2 * M_PI)
                {
                    currentPolarPosition.fi -= 2 * M_PI;
                }

                m_state = PrinterState::PRINTING;
            }
            break;
        }

        case PrinterState::PRINTING:
        {
            if(rTicksCounter==0 && fiTicksCounter==0)
            {
                m_state = PrinterState::SET_STEP;            
            }

            if(fabs(targetPolarPosition.r-currentPolarPosition.r) < 0.25 && fabs(targetPolarPosition.fi-currentPolarPosition.fi) < 0.5 * 2* M_PI/360)
            {
                rTicksCounter = 0;
                fiTicksCounter = 0;
                currentPosition = Coord::convertPolarToDecart(currentPolarPosition);
                m_state = PrinterState::HANDLE_COMMAND;
            }
            break;
        }

        case PrinterState::SEARCH_FI_ZERO:
        {
            if(fiCenterTrigger)
            {
                setStep(-rMoveDiapason * 1.2, 0, 15); // 20% margin
                m_state = PrinterState::SEARCH_R_ZERO;

                printf("Fi zeroing\r\n");
            }
            break;
        }

        case PrinterState::SEARCH_R_ZERO:
        {
            if(rCenterTrigger)
            {
                stop();
//                NVIC_DisableIRQ(EXTI15_10_IRQn);

                printf("R zeroing\r\n");

                // TODO: find real center from sensor data, not fixed move
                rTicksCounter = lengthToMotorTicks(7.5); //setStep()?
                m_state = PrinterState::CORRECTING_CENTER;
            }
            break;
        }

        case PrinterState::CORRECTING_CENTER:
        {
            if(rTicksCounter==0)
            {
                currentPolarPosition.r = 0;
                currentPolarPosition.fi = 0;

                setStep(0, coordSysRotation, 30);
                m_state = PrinterState::IDLE;
            }
            break;
        }

        case PrinterState::ROTATE_COORD_SYS:
        {
            if(fiTicksCounter)
            {
                m_state = PrinterState::IDLE;
            }
            break;
        }

        default: {}
    }
}

void Printer::setStep(double_t dR, double_t dFi, double_t stepTimeInSec)
{
    bool rDirection;

    rTicksCounter = lengthToMotorTicks(abs(dR));
    int32_t correctionTicks = lengthToMotorTicks(abs(dFi) * errRonRadian);

    if(dFi > 0) // counter clokwise, R decrease
    {
        printerPins->fiDirState(Pins::PinState::RESET);
        if(dR >= 0)
        {
            rDirection = 0;
            rTicksCounter += correctionTicks;
        }
        else
        {
            int32_t resultTicks = rTicksCounter - correctionTicks;

            if(resultTicks > 0) rDirection = 1;
            else rDirection = 0;

            rTicksCounter = abs(resultTicks);
        }
    }
    else // clockwise, R increase
    {
        printerPins->fiDirState(Pins::PinState::SET);
        if(dR >= 0)
        {
            int32_t resultTicks = rTicksCounter - correctionTicks;

            if(resultTicks > 0) rDirection = 0;
            else rDirection = 1;

            rTicksCounter = abs(resultTicks);
        }
        else
        {
            rDirection = 1;
            rTicksCounter += correctionTicks;
        }
    }

   printerPins->rDirState(static_cast<Pins::PinState>(rDirection));

    fiTicksCounter = radiansToMotorTicks(abs(dFi));

    float_t rPeriod = stepTimeInSec/rTicksCounter;
    float_t fiPeriod = stepTimeInSec/fiTicksCounter;

    setTIMPeriods(rPeriod, fiPeriod);
}

uint32_t Printer::radiansToMotorTicks(double_t radians)
{
    return round(fiTicksCoef * radians);
}

uint32_t Printer::lengthToMotorTicks(double_t length)
{
    return round(rTicksCoef * length);
}

void Printer::setTIMPeriods(float_t rStepTime, float_t fiStepTime)
{
    uint32_t timerRPeriod = round(rStepTime * timerPrescaler / 2);
    uint32_t timerFiPeriod = round(fiStepTime * timerPrescaler / 2);
    
    if(timerRPeriod < minRPeriod)
    {
        float_t fiCorretionCoef;
        fiCorretionCoef = (float_t)minRPeriod / (float_t)timerRPeriod;
        timerRPeriod = minRPeriod;
        timerFiPeriod *=  fiCorretionCoef;
        printf("Fi correction, coef: %lf\r\n", fiCorretionCoef);
    }

    if(timerFiPeriod < minFiPeriod)
    {
        float_t rCorretionCoef;
        rCorretionCoef = (float_t)minFiPeriod / (float_t)timerFiPeriod;
        timerFiPeriod = minFiPeriod;
        timerRPeriod *=  rCorretionCoef;
        printf("R correction, coef: %lf\r\n", rCorretionCoef);
    }

    rTimer->setInterval(timerRPeriod);
    fiTimer->setInterval(timerFiPeriod);

    //printf("TIM periods r:%d, fi:%d\r\n", timerRPeriod, timerFiPeriod);
}

void Printer::makeRStep()
{
    if(m_state == PrinterState::PRINTING || m_state == PrinterState::SEARCH_FI_ZERO || m_state == PrinterState::SEARCH_R_ZERO || m_state == PrinterState::CORRECTING_CENTER)
    {
        if(rTicksCounter>0)
        {
            if(printerPins->getRStep() == Pins::PinState::RESET)
            {

                printerPins->rStepState(Pins::PinState::SET);

                if(printerPins->getRDir() == Pins::PinState::RESET)
                {
                    currentPolarPosition.r += mmOnRTick;
                }
                else
                {
                    currentPolarPosition.r -= mmOnRTick;
                }
            }
            else
            {
                printerPins->rStepState(Pins::PinState::RESET);
                rTicksCounter--;
            }
        }
    }
}

void Printer::makeFiStep()
{
    if(m_state == PrinterState::PRINTING || m_state == PrinterState::SEARCH_FI_ZERO || m_state == PrinterState::SEARCH_R_ZERO || m_state == PrinterState::CORRECTING_CENTER)
    {
        if(fiTicksCounter>0)
        {
            if(printerPins->getFiStep() == Pins::PinState::RESET)
            {
                printerPins->fiStepState(Pins::PinState::SET);

                if(printerPins->getFiDir() == Pins::PinState::RESET)
                {
                    currentPolarPosition.fi += radOnFiTick;
                    currentPolarPosition.r -= errRonTick;
                }
                else
                {
                    currentPolarPosition.fi -= radOnFiTick;
                    currentPolarPosition.r += errRonTick;
                }
            }
            else
            {
                printerPins->fiStepState(Pins::PinState::RESET);
                fiTicksCounter--;
            }
        }
    }
}

void Printer::pauseThread()
{
    if(rTimer && fiTimer)
    {
        rTimer->stop();
        fiTimer->stop();
    }
}

void Printer::resumeThread()
{
    if(rTimer && fiTimer)
    {
        rTimer->start();
        fiTimer->start();
    }
}

void Printer::stop()
{
    rTicksCounter = 0;
    fiTicksCounter = 0;
}

bool Printer::isPrinterFree()
{
    return m_state == PrinterState::IDLE;
}

bool Printer::isQueueFull()
{
    return m_printJob.size() >= printerQueueSize;
}