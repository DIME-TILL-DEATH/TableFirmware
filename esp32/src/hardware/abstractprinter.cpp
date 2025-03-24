#include <math.h>

#include "esp_log.h"
#include "abstractprinter.hpp"

#include "filemanager/settings.hpp"

AbstractPrinter::AbstractPrinter()
{
    m_state = PrinterState::IDLE;
    m_previousState = m_state;

    speed = 25;
    printScaleCoef = 1;

    currentPosition.x = 0;
    currentPosition.y = 0;
}

void AbstractPrinter::initTimers(gptimer_alarm_cb_t firstCoordTimerCb, gptimer_alarm_cb_t secondCoordTimerCb)
{  
    firstCoordTimer = new IntervalTimer(firstCoordTimerCb);
    secondCoordTimer = new IntervalTimer(secondCoordTimerCb);  
}

void AbstractPrinter::initPins(gpio_isr_t endStops_cb)
{
    printerPins = new Pins::PrinterPins(endStops_cb);
}

void AbstractPrinter::setState(PrinterState newState)
{
    if(newState != m_state) 
    {
        m_previousState = m_state;
        m_state = newState;
    }
}

void AbstractPrinter::returnToPreviousState()
{
    m_state = m_previousState;
}

void AbstractPrinter::setNextCommand(GCode::GAbstractComm* command)
{
    nextComm = command;
}

void AbstractPrinter::setTIMPeriods(float_t firstMotorStepTime, float_t secondMotorStepTime)
{
    uint32_t timerFirstMotorPeriod = round(firstMotorStepTime * timerPrescaler / 2);
    uint32_t timerSecondMotorPeriod = round(secondMotorStepTime * timerPrescaler / 2);  
    
    if(timerSecondMotorPeriod < minRPeriod)
    {
        float_t fiCorretionCoef;
        fiCorretionCoef = (float_t)minRPeriod / (float_t)timerSecondMotorPeriod;
        timerSecondMotorPeriod = minRPeriod;
        timerFirstMotorPeriod *=  fiCorretionCoef;
        //printf("R min timer!(fi timer: %d, r timer: %d), Fi corretion coef: %lf\r\n", timerFiPeriod, timerRPeriod, fiCorretionCoef);
    }

    if(timerFirstMotorPeriod < minFiPeriod)
    {
        float_t rCorretionCoef;
        rCorretionCoef = (float_t)minFiPeriod / (float_t)timerFirstMotorPeriod;
        timerFirstMotorPeriod = minFiPeriod;
        timerSecondMotorPeriod *=  rCorretionCoef;
        //printf("Fi min timer!(fi timer: %d, r timer: %d), R correction, coef: %lf\r\n", timerFiPeriod, timerRPeriod, rCorretionCoef);
    }

    firstCoordTimer->setInterval(timerFirstMotorPeriod);
    secondCoordTimer->setInterval(timerSecondMotorPeriod); 
}

void AbstractPrinter::pauseThread()
{
    if(secondCoordTimer && firstCoordTimer)
    {
        secondCoordTimer->stop();
        firstCoordTimer->stop();
    }
}

void AbstractPrinter::resumeThread()
{
    if(secondCoordTimer && firstCoordTimer)
    {
        secondCoordTimer->start();
        firstCoordTimer->start();
    }
}

void AbstractPrinter::abortPoint()
{
    secondMotorTicksCounter = 0;
    firstMotorTicksCounter = 0;
}

void AbstractPrinter::stop()
{
    abortPoint();
    setState(PrinterState::IDLE);
}

void AbstractPrinter::pause(uint32_t time)
{
    setState(PrinterState::PAUSE);
    pauseCounter = time;
}

void AbstractPrinter::pauseResume()
{
    if(m_state == PrinterState::PAUSE)
    {
        returnToPreviousState();
    }
    else
    {
        pause(infinitePause);
    }
    
}

bool AbstractPrinter::isPrinterFree()
{
    return m_state == PrinterState::IDLE;
}