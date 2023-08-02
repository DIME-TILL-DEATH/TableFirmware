#include <math.h>

#include "Printer.hpp"

Printer::Printer()
{
    m_state = PrinterState::IDLE;

    currentPosition.x = 0;
    currentPosition.y = 0;

    timersInit();
    pinsInit();

//    fiTicksCoef = (((float_t)gear2TeethCount/(float_t)gear1TeethCount) * motorRoundTicks) / (2 * M_PI);
    fiTicksCoef = motorRoundTicks / (2 * M_PI);
}

void Printer::timersInit()
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);

    // Make step timers
    TIM_TimeBaseStructure.TIM_Period = 10000;
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

    // Update point timer. 0.01mm timer.
    // timer period setting print speed.
    TIM_TimeBaseStructure.TIM_Prescaler = TIM2->PSC * 10;
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
    // switch-case
    if(m_state == PrinterState::IDLE)
    {
        if(m_printJob.size()>0)
        {
            m_state = PrinterState::SETTLING;
        }
    }
    else if (m_state == PrinterState::SETTLING)
    {
        if(m_printJob.size()>0)
        {
            nextPoint = m_printJob.front();

            m_printJob.pop();

            float_t l = sqrt(pow((nextPoint.x - currentPosition.x), 2)
                            + pow((nextPoint.y - currentPosition.y), 2));

            float_t timeInSec = l/speed;

            Coord::PolarPoint curPolarPoint = Coord::convertDecartToPolar(currentPosition);
            Coord::PolarPoint tarPolarPoint = Coord::convertDecartToPolar(nextPoint);

            float_t deltaR = tarPolarPoint.r - curPolarPoint.r;
            float_t deltaFi = tarPolarPoint.fi - curPolarPoint.fi;

            if(deltaR > 0) GPIO_WriteBit(pinRDir.port, pinRDir.pin, Bit_SET);
            else GPIO_WriteBit(pinRDir.port, pinRDir.pin, Bit_RESET);;

            uint8_t dirFi;
            if(deltaFi >0)
                {
                GPIO_WriteBit(pinFiDir.port, pinFiDir.pin, Bit_SET);
                dirFi = 1;
                }
            else {
                GPIO_WriteBit(pinFiDir.port, pinFiDir.pin, Bit_RESET);
                dirFi = 0;
            }

            deltaFi = abs(deltaFi);
            deltaR = abs(deltaR);

            rTicksCounter = deltaR * rTicksCoef; ///targetSteps;
            fiTicksCounter = radiansToMotorTicks(deltaFi);

            float_t rPeriod = timeInSec/rTicksCounter;
            float_t fiPeriod = timeInSec/fiTicksCounter;

            printf("=time: %lf, tar Fi:%lf\r\n", timeInSec, tarPolarPoint.fi * 360 / (M_PI * 2));
            printf("==r ticks=%d, r period=%lf\r\n", rTicksCounter, rPeriod);
            printf("==fi ticks=%d, fi period=%lf\r\n", fiTicksCounter, fiPeriod);

            setRStepPeriod(rPeriod);
            setFiStepPeriod(fiPeriod);

            m_state = PrinterState::PRINTING;
        }
        else m_state = PrinterState::IDLE;
    }
    else if(m_state == PrinterState::PRINTING)
    {
        if(rTicksCounter==0 && fiTicksCounter==0)
        {
            m_state = PrinterState::SETTLING;
            currentPosition = nextPoint;
        }
    }
//    else if(m_state == PrinterState::FINISHED_STEP)
//    {
//        if(targetSteps>0)
//        {
//            rTicksCounter = rStepTicks;
//            fiTicksCounter = fiStepTicks;
//
//            targetSteps--;
//        }
//        else
//        {
//            currentPosition = nextPoint;
//            m_state = PrinterState::SETTLING;
//        }
//    }
}

uint32_t Printer::radiansToMotorTicks(float_t radians)
{
    return round(fiTicksCoef * radians) * uTicks;
}

void Printer::setRStepPeriod(float_t timeInSec)
{
    uint32_t timerPeriod = round(timeInSec * timerPrescaler / 2);

    printf("TIM2 autoreload=%d\r\n", timerPeriod);
    TIM_SetAutoreload(TIM2, timerPeriod);
    TIM_SetCounter(TIM2, 0);
}

void Printer::setFiStepPeriod(float_t timeInSec)
{
    uint32_t timerPeriod = round(timeInSec * timerPrescaler / 2);
    printf("TIM3 autoreload=%d\r\n", timerPeriod);
    TIM_SetAutoreload(TIM3, timerPeriod);
    TIM_SetCounter(TIM3, 0);
}

void Printer::makeRStep()
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

void Printer::makeFiStep()
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
