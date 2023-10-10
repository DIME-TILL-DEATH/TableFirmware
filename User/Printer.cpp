#include <math.h>

#include "Printer.hpp"

Printer::Printer()
{
    m_state = PrinterState::IDLE;

    currentPosition.x = 0;
    currentPosition.y = 0;

    timersInit();
    pinsInit();

    rTicksCoef = (double_t)motorRoundTicks / (double_t)(rGearStep * rGearTeethCount);
    fiTicksCoef = (((double_t)fiGear2TeethCount/(double_t)fiGear1TeethCount) * motorRoundTicks) / (2 * M_PI);
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

    TIM_TimeBaseStructure.TIM_Prescaler = TIM2->PSC * 5;
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

    pinRSensor.pin = GPIO_Pin_13;
    pinRSensor.port = GPIOB;
    GPIO_InitStructure.GPIO_Pin = pinRSensor.pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(pinRSensor.port, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStructure);

    pinFiSensor.pin = GPIO_Pin_14;
    pinFiSensor.port = GPIOB;
    GPIO_InitStructure.GPIO_Pin = pinRSensor.pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(pinRSensor.port, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
    EXTI_InitStructure.EXTI_Line = EXTI_Line14;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStructure);
}

bool Printer::findCenter()
{
    NVIC_DisableIRQ(TIM2_IRQn);
    NVIC_DisableIRQ(TIM3_IRQn);
    NVIC_DisableIRQ(TIM4_IRQn);

    TIM_SetAutoreload(TIM2, 500);
    TIM_SetCounter(TIM2, 0);
    TIM_SetAutoreload(TIM3, 500);
    TIM_SetCounter(TIM3, 0);

    printf("Finding table center...\r\n");
    currentPosition.x = 0;
    currentPosition.y = 0;

    fiCenterTrigger = false;
    rCenterTrigger = false;

    if(GPIO_ReadInputDataBit(pinFiSensor.port, pinFiSensor.pin) == Bit_SET)
    {
        fiCenterTrigger = true;
    }

    if(GPIO_ReadInputDataBit(pinRSensor.port, pinRSensor.pin) == Bit_SET)
    {
        rCenterTrigger = true;
    }

    m_state = PrinterState::SEARCH_FI_ZERO;

    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_EnableIRQ(TIM4_IRQn);

    NVIC_EnableIRQ(EXTI15_10_IRQn);

    return true;
}

PrinterState Printer::state()
{
    return m_state;
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

                m_printJob.pop();

                fiSumTicks = 0;

                if(targetPosition.x == currentPosition.x && targetPosition.y == targetPosition.y) return; //skip point

                float_t deltaX = targetPosition.x - currentPosition.x;
                float_t deltaY = targetPosition.y - currentPosition.y;

                float_t lineLength = sqrt(pow((deltaX), 2) + pow((deltaY), 2));
                float_t sizeCoef = lineLength/stepSize;
                stepX = deltaX / sizeCoef;
                stepY = deltaY / sizeCoef;
                stepTime = sizeCoef / speed;

                printf("current pos:(%lf, %lf), target pos(%lf, %lf)\r\n", currentPosition.x, currentPosition.y, targetPosition.x, targetPosition.y);
                printf("length: %lf, coef: %lf, stepX: %lf, stepY: %lf\r\n", lineLength, sizeCoef, stepX, stepY);

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
//                printf("===last fi sum ticks: %d\r\n", fiSumTicks);
                m_state = PrinterState::SET_POINT;
            }
            else
            {
                GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_SET);
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
                        printf("!!!!!fi correction! deltaFi before: %lf => \r\n", deltaFi * 360 / (M_PI * 2));
                        deltaFi = (-1) * copysign(2 * M_PI - fabs(deltaFi), deltaFi);
                    }
                }

                if(deltaR > 0) GPIO_WriteBit(pinRDir.port, pinRDir.pin, Bit_SET);
                else GPIO_WriteBit(pinRDir.port, pinRDir.pin, Bit_RESET);;

                if(deltaFi >0) GPIO_WriteBit(pinFiDir.port, pinFiDir.pin, Bit_SET);
                else GPIO_WriteBit(pinFiDir.port, pinFiDir.pin, Bit_RESET);


//                printf("deltaR: %lf, deltaFi: %lf\r\n", deltaR * 360 / (M_PI * 2), deltaFi * 360 / (M_PI * 2));

                deltaFi = abs(deltaFi);
                deltaR = abs(deltaR);

                rTicksCounter = lengthToMotorTicks(deltaR);
                fiTicksCounter = radiansToMotorTicks(deltaFi);

//                fiSumTicks += fiTicksCounter;

                float_t rPeriod = stepTime/rTicksCounter;
                float_t fiPeriod = stepTime/fiTicksCounter;
//
                setRStepPeriod(rPeriod);
                setFiStepPeriod(fiPeriod);
                currentPosition.x += stepX;
                currentPosition.y += stepY;


//                printf("==curXY(%f, %f)/curRFi(%f, %f))--r tcks=%d, fi tck=%d\r\n", currentPosition.x, currentPosition.y,
//                                                                                        curPolarPoint.r, curPolarPoint.fi * 360 / (M_PI * 2),
//                                                                                        rTicksCounter, fiTicksCounter);


                m_state = PrinterState::PRINTING;
                GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_RESET);
            }
            break;
        }

        case PrinterState::PRINTING:
        {
            if(rTicksCounter==0 && fiTicksCounter==0)
            {
                m_state = PrinterState::SET_STEP;
            }
            break;
        }

        case PrinterState::SEARCH_FI_ZERO:
        {
            if(!fiCenterTrigger)
            {
                fiTicksCounter = radiansToMotorTicks(M_PI);
            }
            else
            {
                fiTicksCounter = 0;
                m_state = PrinterState::SEARCH_R_ZERO;
            }
            break;
        }

        case PrinterState::SEARCH_R_ZERO:
        {
            if(!rCenterTrigger)
            {
                // 扶忘扭把忘志抖快扶我快???
                rTicksCounter = lengthToMotorTicks(20);
            }
            else
            {
                rTicksCounter = 0;
                m_state = PrinterState::IDLE;
                NVIC_DisableIRQ(EXTI15_10_IRQn);
            }
            break;
        }
    }
}

uint32_t Printer::radiansToMotorTicks(double_t radians)
{
    return round(fiTicksCoef * radians * uTicks);
}

uint32_t Printer::lengthToMotorTicks(double_t length)
{
    return round(rTicksCoef * length * uTicks);
}

void Printer::setRStepPeriod(double_t timeInSec)
{
    uint32_t timerPeriod = round(timeInSec * timerPrescaler / 2);

//    printf("TIM2 autoreload=%d\r\n", timerPeriod);
    TIM_SetAutoreload(TIM2, timerPeriod);
    TIM_SetCounter(TIM2, 0);
}

void Printer::setFiStepPeriod(double_t timeInSec)
{
    uint32_t timerPeriod = round(timeInSec * timerPrescaler / 2);

    if(timerPeriod < 700)
    {
        printf("TIM3 correct autoreload counted: %d, settled: 750\r\n", timerPeriod);
        timerPeriod = 750;
    }
    TIM_SetAutoreload(TIM3, timerPeriod);
    TIM_SetCounter(TIM3, 0);
}

void Printer::makeRStep()
{
    if(m_state == PrinterState::PRINTING || m_state == PrinterState::SEARCH_R_ZERO)
    {
        if(rTicksCounter>0)
        {
            if(GPIO_ReadOutputDataBit(pinRStep.port, pinRStep.pin) == Bit_RESET)
            {
                GPIO_WriteBit(pinRStep.port, pinRStep.pin, Bit_SET);
            }
            else
            {
                GPIO_WriteBit(pinRStep.port, pinRStep.pin, Bit_RESET);
                rTicksCounter--;
            }
        }
    }
}

void Printer::makeFiStep()
{
    if(m_state == PrinterState::PRINTING || m_state == PrinterState::SEARCH_FI_ZERO)
    {
        if(fiTicksCounter>0)
        {
            if(GPIO_ReadOutputDataBit(pinFiStep.port, pinFiStep.pin) == Bit_RESET)
            {
                GPIO_WriteBit(pinFiStep.port, pinFiStep.pin, Bit_SET);
            }
            else
            {
                GPIO_WriteBit(pinFiStep.port, pinFiStep.pin, Bit_RESET);
                fiTicksCounter--;
            }
        }
    }
}
