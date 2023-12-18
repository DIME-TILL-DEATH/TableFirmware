#ifndef USER_PRINTER_HPP_
#define USER_PRINTER_HPP_

#include <vector>
#include <queue>

#include "intervaltimer.hpp"
#include "printerpins.hpp"

#include "geometry/coordinates.hpp"

#include "gcode/gabstractcomm.hpp"
#include "gcode/m51comm.hpp"
#include "gcode/g1comm.hpp"
#include "gcode/g4comm.hpp"

// #define EXTI_RSENS_LINE EXTI_Line10
// #define EXTI_FISENS_LINE EXTI_Line13

typedef enum
{
    ERROR = 0,
    IDLE,
    SET_POINT,
    PRINTING,
    SET_STEP,
    SEARCH_R_ZERO,
    SEARCH_FI_ZERO,
    CORRECTING_CENTER
}PrinterState;

class Printer
{
public:
    Printer();

    void initTimers(gptimer_alarm_cb_t rTimerCb,
                    gptimer_alarm_cb_t fiTimerCb);
    void initPins();

    void findCenter();

    void pushPrintCommand(GCode::GAbstractComm* command);
    void printRoutine();

    void makeRStep();
    void makeFiStep();

    void trigFiZero() {fiCenterTrigger = true;};
    void trigRZero() {rCenterTrigger = true;};

    void pauseThread();
    void resumeThread();

    bool isPrinterFree();
    bool isQueueFull();
private:
    typedef enum
    {
        ERROR = 0,
        IDLE,
        HANDLE_COMMAND,
        SET_POINT,
        SET_STEP,
        PRINTING,
        SEARCH_R_ZERO,
        SEARCH_FI_ZERO,
        CORRECTING_CENTER,
        ROTATE_COORD_SYS
    }PrinterState;

    Pins::PrinterPins* printerPins;

    IntervalTimer* rTimer;
    IntervalTimer* fiTimer;

    PrinterState m_state;
    std::queue<GCode::GAbstractComm*> m_printJob;

    Coord::DecartPoint currentPosition{0, 0};
    Coord::DecartPoint targetPosition{0, 0};

    Coord::PolarPoint currentPolarPosition{0, 0};
    Coord::PolarPoint targetPolarPosition{0, 0};

    uint32_t rTicksCounter{0};
    uint32_t fiTicksCounter{0};

    uint32_t pointNum{0};

    double_t stepX;
    double_t stepY;
    double_t stepTime;

    // PrinterPin pinRStep;
    // PrinterPin pinRDir; // RESET - R>0
    // PrinterPin pinRSensor;

    // PrinterPin pinFiStep;
    // PrinterPin pinFiDir;  // SET - clockwise direction
    // PrinterPin pinFiSensor;

    bool fiCenterTrigger = false;
    bool rCenterTrigger = false;

    float_t speed = 40;//7.5;
    float_t coordSysRotation = M_PI_2;

    void setTIMPeriods(float_t rStepTime, float_t fiStepTime);

    uint32_t radiansToMotorTicks(double_t radians);
    uint32_t lengthToMotorTicks(double_t length);

    void setStep(double_t dR, double_t dFi, double_t stepTime);
    void stop();

    static constexpr uint16_t printerQueueSize = 32;

    static constexpr uint32_t timerPrescaler = 1000000; // 1us
    static constexpr uint16_t minRPeriod = 100;
    static constexpr uint16_t minFiPeriod = 250;

    static constexpr float_t printScaleCoef = 0.7;

    static constexpr float_t stepSize = 1;

    static constexpr uint16_t rMoveDiapason = 270;

    static constexpr uint16_t uTicks = 16;
    static constexpr uint16_t motorRoundTicks = 200;

    static constexpr uint16_t rGearTeethCount = 20;
    static constexpr uint16_t rGearStep = 2;
    static constexpr float_t rTicksCoef = (float_t)uTicks * (float_t)motorRoundTicks / (float_t)(rGearStep * rGearTeethCount);

    static constexpr uint16_t fiGear1TeethCount = 20;
    static constexpr uint16_t fiGear2TeethCount = 160;//202;
    static constexpr float_t fiTicksCoef = (float_t)uTicks * (((float_t)fiGear2TeethCount/(float_t)fiGear1TeethCount) * motorRoundTicks) / (2 * M_PI);

    static constexpr float_t errRonRadian = fiGear1TeethCount*rGearStep/(2*M_PI); // 40mm error in R on 2pi (360) rotation
                                                // clockwise direction: -R

    static constexpr float_t mmOnRTick = 1/rTicksCoef;
    static constexpr float_t radOnFiTick = 1/fiTicksCoef;
    static constexpr float_t errRonTick = errRonRadian/fiTicksCoef;
};

#endif /* USER_PRINTER_HPP_ */
