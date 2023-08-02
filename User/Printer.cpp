#include <math.h>

#include "Printer.hpp"

Printer::Printer()
{
    m_state = PrinterState::IDLE;

    currentPosition.x = 0;
    currentPosition.y = 0;

    timersInit();
    pinsInit();

    fiTicksCoef = (((float_t)gear2TeethCount/(float_t)gear1TeethCount) * motorRoundTicks) / (2 * M_PI);
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
    pinRStep.port = GPIOB;
    GPIO_InitStructure.GPIO_Pin = pinRStep.pin;
    GPIO_Init(pinRStep.port, &GPIO_InitStructure);

    pinRStep.pin = GPIO_Pin_4;
    pinRStep.port = GPIOB;
    GPIO_InitStructure.GPIO_Pin = pinRStep.pin;
    GPIO_Init(pinRStep.port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    pinRDir.pin = GPIO_Pin_13;
    pinRDir.port = GPIOD;
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
}

bool Printer::findCenter()
{
    NVIC_DisableIRQ(TIM2_IRQn);
    NVIC_DisableIRQ(TIM3_IRQn);
    NVIC_DisableIRQ(TIM4_IRQn);

    printf("Finding table center...\r\n");
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_EnableIRQ(TIM4_IRQn);

//    currentPosition.x = 0;
//    currentPosition.y = 0;
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
            if((fabs(targetPosition.x - currentPosition.x) <= fabs(stepX)) && (fabs(targetPosition.y - currentPosition.y) <= fabs(stepY)))
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

                if(deltaR > 0) GPIO_WriteBit(pinRDir.port, pinRDir.pin, Bit_SET);
                else GPIO_WriteBit(pinRDir.port, pinRDir.pin, Bit_RESET);;

                if(deltaFi >0) GPIO_WriteBit(pinFiDir.port, pinFiDir.pin, Bit_SET);
                else GPIO_WriteBit(pinFiDir.port, pinFiDir.pin, Bit_RESET);

                deltaFi = abs(deltaFi);
                deltaR = abs(deltaR);

                rTicksCounter = deltaR * rTicksCoef;
                fiTicksCounter = radiansToMotorTicks(deltaFi);

//                fiSumTicks += fiTicksCounter;

                float_t rPeriod = stepTime/rTicksCounter;
                float_t fiPeriod = stepTime/fiTicksCounter;
//
//                printf("++++++time: %lf, tar Fi:%lf\r\n", stepTime, tarPolarPoint.fi * 360 / (M_PI * 2));
//                printf("+++++++ticks=%d, r period=%lf\r\n", rTicksCounter, rPeriod);
//                printf("+++++++ticks=%d, fi period=%lf\r\n", fiTicksCounter, fiPeriod);
//
                setRStepPeriod(rPeriod);
                setFiStepPeriod(fiPeriod);
                currentPosition.x += stepX;
                currentPosition.y += stepY;

//                printf("----deltaR: %lf, deltaFi: %lf\r\n", deltaR * 360 / (M_PI * 2), deltaFi * 360 / (M_PI * 2));
//                printf("==cur: (%lf, %lf)----r ticks=%d, fi ticks=%d\r\n", currentPosition.x, currentPosition.y, rTicksCounter, fiTicksCounter);


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
    }
}

uint32_t Printer::radiansToMotorTicks(double_t radians)
{
    return round(fiTicksCoef * radians * uTicks);
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
//    printf("TIM3 autoreload=%d\r\n", timerPeriod);
    TIM_SetAutoreload(TIM3, timerPeriod);
    TIM_SetCounter(TIM3, 0);
}

void Printer::makeRStep()
{
    if(m_state == PrinterState::PRINTING)
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
    if(m_state == PrinterState::PRINTING)
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
