#ifndef USER_PRINTER_HPP_
#define USER_PRINTER_HPP_

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

class Printer
{
public:
    Printer();

    void initTimers(gptimer_alarm_cb_t rTimerCb,
                    gptimer_alarm_cb_t fiTimerCb);
    void initPins(gpio_isr_t endStops_cb);
    void loadSettings();

    void findCenter();

    void setNextCommand(GCode::GAbstractComm* command);
    void printRoutine();

    void makeRStep();
    void makeFiStep();

    void trigFiZero() {fiCenterTrigger = true;};
    void trigRZero() {rCenterTrigger = true;};

    void setSpeed(float_t newSpeed) {speed = newSpeed;};
    float_t getSpeed() {return speed;};

    void setPauseInterval(uint32_t newPauseInterval) {pauseInterval = newPauseInterval;};
    uint32_t getPauseInterval() {return pauseInterval;};

    uint16_t getFiGear2TeethsCount() {return fiGear2TeethCount;};

    void stop();
    void pause(uint32_t time);
    void pause();
    void resume();

    uint16_t currentPrintPointNum() {return pointNum;}

    bool isPrinterFree();
private:
    constexpr static char TAG[] = "PRINTER";

    typedef enum
    {
        ERROR = 0,
        IDLE,
        HANDLE_COMMAND,
        SET_POINT,
        SET_STEP,
        PRINTING,
        SEARCH_FI_ZERO,
        RESCAN_FI_ZERO,
        SEARCH_R_ZERO,
        RESCAN_R_ZERO,
        CORRECTING_CENTER,
        ROTATE_COORD_SYS,
        PAUSE
    }PrinterState;

    Pins::PrinterPins* printerPins;

    IntervalTimer* rTimer;
    IntervalTimer* fiTimer;

    PrinterState m_state, m_previousState;
    GCode::GAbstractComm* nextComm{nullptr};

    Coord::DecartPoint currentPosition{0, 0};
    Coord::DecartPoint targetPosition{0, 0};

    Coord::PolarPoint currentPolarPosition{0, 0};
    Coord::PolarPoint targetPolarPosition{0, 0};

    uint16_t pauseCounter{0};
    static constexpr uint16_t infinitePause = 0xFFFF;

    uint32_t rTicksCounter{0};
    uint32_t fiTicksCounter{0};

    uint32_t pointNum{0};

    double_t stepX;
    double_t stepY;
    double_t stepTime;

    bool fiCenterTrigger = false;
    bool rCenterTrigger = false;

    float_t speed;
    float_t coordSysRotation;
    float_t printScaleCoef;
    float_t correctionLength{0};

    uint32_t pauseInterval;

    uint16_t rMoveDiapason = 500;

    void abortPoint();

    void pauseThread();
    void resumeThread();

    void setTIMPeriods(float_t rStepTime, float_t fiStepTime);

    uint32_t radiansToMotorTicks(double_t radians);
    uint32_t lengthToMotorTicks(double_t length);

    void setStep(double_t dR, double_t dFi, double_t stepTime);
    void setState(PrinterState newState);
    void returnToPreviousState();

    static constexpr uint16_t printerQueueSize = 32;

    static constexpr uint32_t timerPrescaler = 1000000; // 1us
    static constexpr uint16_t minRPeriod = 100;
    static constexpr uint16_t minFiPeriod = 250;

    static constexpr float_t stepSize = 1;

    static constexpr uint16_t uTicks = 16;
    static constexpr uint16_t motorRoundTicks = 200;

    uint16_t rGearTeethCount = 20;
    static constexpr uint16_t rGearStep = 2;
    float_t rTicksCoef; // = (float_t)uTicks * (float_t)motorRoundTicks / (float_t)(rGearStep * rGearTeethCount);
    void setRGearTeethCount(uint16_t newRGearTeethCount);

    uint16_t fiGear1TeethCount;// = 20;
    uint16_t fiGear2TeethCount;// = 160;//202;
    float_t fiTicksCoef; // = (float_t)uTicks * (((float_t)fiGear2TeethCount/(float_t)fiGear1TeethCount) * motorRoundTicks) / (2 * M_PI);
    float_t errRonRadian; // = fiGear1TeethCount*rGearStep/(2*M_PI); // 40mm error in R on 2pi (360) rotation
                                                // clockwise direction: -R

    void setFiGearTeethCount(uint16_t newFiGear1TeethCount, uint16_t newFiGear2TeethCount);

    float_t mmOnRTick;// = 1/rTicksCoef;
    float_t radOnFiTick;// = 1/fiTicksCoef;
    float_t errRonTick;// = errRonRadian/fiTicksCoef;
    void recountValuesOnTick();
};

#endif /* USER_PRINTER_HPP_ */
