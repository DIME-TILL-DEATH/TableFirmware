#ifndef USER_ABSTRACTPRINTER_HPP_
#define USER_ABSTRACTPRINTER_HPP_

#include <vector>
#include <queue>

#include "projdefines.h"

#include "intervaltimer.hpp"
#include "printerpins.hpp"

#include "geometry/coordinates.hpp"

#include "gcode/gabstractcomm.hpp"
#include "gcode/m51comm.hpp"
#include "gcode/g1comm.hpp"
#include "gcode/g4comm.hpp"

class AbstractPrinter
{
public:
    AbstractPrinter();

    typedef enum
    {
        ERROR = 0,
        IDLE,
        HANDLE_COMMAND,
        SET_POINT,
        SET_STEP,
        PRINTING,
        SEARCH_FIRST_ZERO,
        RESCAN_FIRST_ZERO,
        SEARCH_SECOND_ZERO,
        RESCAN_SECOND_ZERO,
        CORRECTING_CENTER,
        ROTATE_COORD_SYS,
        PAUSE
    }PrinterState;

    void initTimers(gptimer_alarm_cb_t firstCoordTimerCb, gptimer_alarm_cb_t secondCoordTimerCb);
    void initPins(gpio_isr_t endStops_cb);

    void setNextCommand(GCode::GAbstractComm* command);

    virtual void loadSettings() {};
    virtual void findCenter() {};
    virtual void printRoutine() {};

    virtual void firtsMotorMakeStep() {};
    virtual void secondMotorMakeStep() {};
    
    void trigFiZero() {firstCoordEndstopTrigger = true;};
    void trigRZero() {secondCoordEndstopTrigger = true;};

    void setSpeed(float_t newSpeed) {speed = newSpeed;};
    float_t getSpeed() {return speed;};

    void setPauseInterval(uint32_t newPauseInterval) {pauseInterval = newPauseInterval;};
    uint32_t getPauseInterval() {return pauseInterval;};

    void stop();
    void pause(uint32_t time);
    void pauseResume();

    void abortPoint();
    void pauseThread();
    void resumeThread();

    uint16_t currentPrintPointNum() {return pointNum;}

    bool isPrinterFree();

    bool inverseFirstMotor = false;
    bool inverseSecondMotor = false;

protected:
    constexpr static char TAG[] = "PRINTER";

    double_t stepX;
    double_t stepY;
    double_t stepTime;

    Pins::PrinterPins* printerPins;

    IntervalTimer* firstCoordTimer;
    IntervalTimer* secondCoordTimer;

    PrinterState m_state, m_previousState;
    GCode::GAbstractComm* nextComm{nullptr};

    Coord::DecartPoint currentPosition{0, 0};
    Coord::DecartPoint targetPosition{0, 0};

    uint16_t pauseCounter{0};
    static constexpr uint16_t infinitePause = 0xFFFF;

    uint32_t firstMotorTicksCounter{0};
    uint32_t secondMotorTicksCounter{0};

    uint32_t pointNum{0};

    bool firstCoordEndstopTrigger = false;
    bool secondCoordEndstopTrigger = false;


    float_t speed; 
    float_t printScaleCoef;

    uint32_t pauseInterval;

    void setTIMPeriods(float_t xStepTime, float_t yStepTime);

    void setState(PrinterState newState);
    void returnToPreviousState();

    static constexpr uint16_t printerQueueSize = 32;

    static constexpr uint32_t timerPrescaler = 1000000; // 1us
    uint16_t minRPeriod{0};
    uint16_t minFiPeriod{0};

    static constexpr float_t stepSize = 1;

    static constexpr uint16_t uTicks = 16;
    static constexpr uint16_t motorRoundTicks = 200;
};

#endif /* USER_ABSTRACTPRINTER_HPP_ */
