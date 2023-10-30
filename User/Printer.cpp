#include <math.h>

#include "Printer.hpp"

Printer::Printer()
{
    m_state = PrinterState::IDLE;

    currentPosition.x = 0;
    currentPosition.y = 0;

    currentPolarPosition.r = 0;
    currentPolarPosition.fi = 0;

    timersInit();
    pinsInit();
}

void Printer::timersInit()
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);

    // Make step timers
    TIM_TimeBaseStructure.TIM_Period = 500;
    TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/timerPrescaler; // 1us
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM3_IRQn);

    TIM_TimeBaseStructure.TIM_Prescaler = TIM2->PSC;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    NVIC_EnableIRQ(TIM4_IRQn);

}

void Printer::pinsInit()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                           RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;


    pinRStep.pin = GPIO_Pin_4;
    pinRStep.port = GPIOA;
    GPIO_InitStructure.GPIO_Pin = pinRStep.pin;
    GPIO_Init(pinRStep.port, &GPIO_InitStructure);

    pinRDir.pin = GPIO_Pin_5;
    pinRDir.port = GPIOA;
    GPIO_InitStructure.GPIO_Pin = pinRDir.pin;
    GPIO_Init(pinRDir.port, &GPIO_InitStructure);


    pinFiStep.pin = GPIO_Pin_1;
    pinFiStep.port = GPIOB;
    GPIO_InitStructure.GPIO_Pin = pinFiStep.pin;
    GPIO_Init(pinFiStep.port, &GPIO_InitStructure);

    pinFiDir.pin = GPIO_Pin_0;
    pinFiDir.port = GPIOB;
    GPIO_InitStructure.GPIO_Pin = pinFiDir.pin;
    GPIO_Init(pinFiDir.port, &GPIO_InitStructure);

    EXTI_InitTypeDef EXTI_InitStructure = {0};

    pinRSensor.pin = GPIO_Pin_10;
    pinRSensor.port = GPIOB;
    GPIO_InitStructure.GPIO_Pin = pinRSensor.pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(pinRSensor.port, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);
    EXTI_InitStructure.EXTI_Line = EXTI_RSENS_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStructure);

    pinFiSensor.pin = GPIO_Pin_13;
    pinFiSensor.port = GPIOB;
    GPIO_InitStructure.GPIO_Pin = pinRSensor.pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(pinRSensor.port, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
    EXTI_InitStructure.EXTI_Line = EXTI_FISENS_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStructure);
}

void Printer::findCenter()
{
    pauseThread();

    TIM_SetAutoreload(TIM2, 500);
    TIM_SetCounter(TIM2, 0);
    TIM_SetAutoreload(TIM3, 500);
    TIM_SetCounter(TIM3, 0);

    printf("Finding table center...\r\n");
    currentPosition.x = 0;
    currentPosition.y = 0;

    pointNum = 1;

    fiCenterTrigger = false;
    rCenterTrigger = false;

    GPIO_WriteBit(pinFiDir.port, pinFiDir.pin, Bit_SET);
    GPIO_WriteBit(pinRDir.port, pinRDir.pin, Bit_RESET);

    m_state = PrinterState::SEARCH_FI_ZERO;

    if(GPIO_ReadInputDataBit(pinFiSensor.port, pinFiSensor.pin) == Bit_SET)
    {
        fiCenterTrigger = true;
    }
    else
    {
        setStep(0, -2*M_PI, 30);
    }

    if(GPIO_ReadInputDataBit(pinRSensor.port, pinRSensor.pin) == Bit_RESET)
    {
        rCenterTrigger = true;
    }

    EXTI_ClearITPendingBit(EXTI_FISENS_LINE | EXTI_RSENS_LINE);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    resumeThread();
}

void Printer::pushPrintPoint(const Coord::DecartPoint& printPoint)
{
        m_printJob.push(printPoint);
}

void Printer::printRoutine()
{
    switch(m_state)
    {
        case PrinterState::IDLE:
        {
            if(m_printJob.size()>0)
            {
                m_state = PrinterState::SET_POINT;
            }
            break;
        }

        case PrinterState::SET_POINT:
        {

            if(m_printJob.size()>0)
            {
                targetPosition = m_printJob.front();

                targetPosition.x = targetPosition.x * printScaleCoef;
                targetPosition.y = targetPosition.y * printScaleCoef;

                m_printJob.pop();

                if(targetPosition.x == currentPosition.x && currentPosition.y == targetPosition.y) return; //skip point

                float_t deltaX = targetPosition.x - currentPosition.x;
                float_t deltaY = targetPosition.y - currentPosition.y;
                targetPolarPosition = Coord::convertDecartToPolar(targetPosition);

                float_t lineLength = sqrt(pow((deltaX), 2) + pow((deltaY), 2));
                float_t steps = lineLength/stepSize;
                stepX = deltaX / steps;
                stepY = deltaY / steps;

                stepTime =  (sqrt(pow((stepX), 2) + pow((stepY), 2))) / speed;

                // INFO block==========
//                printf("Point num: %d\r\n", pointNum);
//                printf("DECART: current(%lf, %lf), target(%lf, %lf)\r\n", currentPosition.x, currentPosition.y, targetPosition.x, targetPosition.y);
//                printf("length: %lf, steps: %lf, stepX: %lf, stepY: %lf\r\n", lineLength, steps, stepX, stepY);
//                printf("POLAR: current(%lf, %lf), target(%lf, %lf)\r\n", currentPolarPosition.r, currentPolarPosition.fi* 360 / (M_PI * 2), targetPolarPosition.r, targetPolarPosition.fi* 360 / (M_PI * 2));
//                printf("\r\n");
                //=================================

                pointNum++;

                m_state = PrinterState::SET_STEP;
            }
            else
            {
                m_state = PrinterState::IDLE;
            }
            break;
        }

        case PrinterState::SET_STEP:
        {
            double_t minErrX = (stepX == 0) ? stepSize : fabs(stepX);
            double_t minErrY = (stepY == 0) ? stepSize : fabs(stepY);

            if((fabs(targetPosition.x - currentPosition.x) < minErrX) && (fabs(targetPosition.y - currentPosition.y) < minErrY))
            {
                m_state = PrinterState::SET_POINT;
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
                m_state = PrinterState::SET_POINT;
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
                NVIC_DisableIRQ(EXTI15_10_IRQn);

                printf("R zeroing\r\n");

                rTicksCounter = lengthToMotorTicks(7.5); //setStep()?
                m_state = PrinterState::CORRECTING_CENTER;
            }
            break;
        }

        case PrinterState::CORRECTING_CENTER:
        {
            if(rTicksCounter==0)
            {
                m_state = PrinterState::IDLE;
                currentPolarPosition.r = 0;
                currentPolarPosition.fi = 0;
            }
            break;
        }
    }
}

void Printer::setStep(double_t dR, double_t dFi, double_t stepTimeInSec)
{
    bool rDirection;

    rTicksCounter = lengthToMotorTicks(abs(dR));
    int32_t correctionTicks = lengthToMotorTicks(abs(dFi) * errRonRadian);

    if(dFi > 0) // counter clokwise, R decrease
    {
        GPIO_WriteBit(pinFiDir.port, pinFiDir.pin, Bit_RESET);
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
        GPIO_WriteBit(pinFiDir.port, pinFiDir.pin, Bit_SET);
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

    GPIO_WriteBit(pinRDir.port, pinRDir.pin, (BitAction)rDirection);

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

void Printer::setTIMPeriods(double_t rStepTime, double_t fiStepTime)
{
    uint32_t timerRPeriod = round(rStepTime * timerPrescaler / 2);
    uint32_t timerFiPeriod = round(fiStepTime * timerPrescaler / 2);

    if(timerRPeriod < minRPeriod)
    {
        double_t fiCorretionCoef;
        fiCorretionCoef = (double_t)minRPeriod / (double_t)timerRPeriod;
        timerRPeriod = minRPeriod;
        timerFiPeriod *=  fiCorretionCoef;
    }

    if(timerFiPeriod < minFiPeriod)
    {
        double_t rCorretionCoef;
        rCorretionCoef = (double_t)minFiPeriod / (double_t)timerFiPeriod;
        timerFiPeriod = minFiPeriod;
        timerRPeriod *=  rCorretionCoef;
    }

    TIM_SetAutoreload(TIM2, timerRPeriod);
    TIM_SetCounter(TIM2, 0);
    TIM_SetAutoreload(TIM3, timerFiPeriod);
    TIM_SetCounter(TIM3, 0);
}

void Printer::makeRStep()
{
    if(m_state == PrinterState::PRINTING || m_state == PrinterState::SEARCH_FI_ZERO || m_state == PrinterState::SEARCH_R_ZERO || m_state == PrinterState::CORRECTING_CENTER)
    {
        if(rTicksCounter>0)
        {
            if(GPIO_ReadOutputDataBit(pinRStep.port, pinRStep.pin) == Bit_RESET)
            {
                GPIO_WriteBit(pinRStep.port, pinRStep.pin, Bit_SET);

                if(GPIO_ReadOutputDataBit(pinRDir.port, pinRDir.pin) == Bit_RESET)
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
                GPIO_WriteBit(pinRStep.port, pinRStep.pin, Bit_RESET);
                rTicksCounter--;
            }
        }

        if(rTicksCounter==0)
        {
            printRoutine();
        }
    }
}

void Printer::makeFiStep()
{
    if(m_state == PrinterState::PRINTING || m_state == PrinterState::SEARCH_FI_ZERO || m_state == PrinterState::SEARCH_R_ZERO || m_state == PrinterState::CORRECTING_CENTER)
    {
        if(fiTicksCounter>0)
        {
            if(GPIO_ReadOutputDataBit(pinFiStep.port, pinFiStep.pin) == Bit_RESET)
            {
                GPIO_WriteBit(pinFiStep.port, pinFiStep.pin, Bit_SET);

                if(GPIO_ReadOutputDataBit(pinFiDir.port, pinFiDir.pin) == Bit_RESET)
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
                GPIO_WriteBit(pinFiStep.port, pinFiStep.pin, Bit_RESET);
                fiTicksCounter--;
            }
        }

        if(fiTicksCounter==0)
        {
            printRoutine();
        }
    }
}

void Printer::pauseThread()
{
    NVIC_DisableIRQ(TIM2_IRQn);
    NVIC_DisableIRQ(TIM3_IRQn);
    NVIC_DisableIRQ(TIM4_IRQn);
}

void Printer::resumeThread()
{
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_EnableIRQ(TIM4_IRQn);
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
