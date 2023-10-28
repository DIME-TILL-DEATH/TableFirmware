#include <math.h>

#include "PrinterHardware.hpp"

PrinterHardware::PrinterHardware()
{

    m_state = PrinterHWState::IDLE;

    currentPosition.x = 0;
    currentPosition.y = 0;

    currentPolarPosition.r = 0;
    currentPolarPosition.fi = 0;

    timersInit();
    pinsInit();
}

void PrinterHardware::timersInit()
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

void PrinterHardware::pinsInit()
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
void PrinterHardware::pause()
{

}

void PrinterHardware::resume()
{

}

void PrinterHardware::stop()
{

}
