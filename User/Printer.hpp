#ifndef USER_PRINTER_HPP_
#define USER_PRINTER_HPP_

#include <vector>
#include <queue>

#include "debug.h"

#include "Coordinates.hpp"

#define EXTI_RSENS_LINE EXTI_Line10
#define EXTI_FISENS_LINE EXTI_Line13

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

typedef struct
{
    GPIO_TypeDef* port;
    uint16_t pin;
}PrinterPin;

class Printer
{
public:
    Printer();

    void findCenter();

    void pushPrintPoint(const Coord::DecartPoint& printPoint);
    void printRoutine();

    void makeRStep();
    void makeFiStep();

    void trigFiZero() {fiCenterTrigger = true;};
    void trigRZero() {rCenterTrigger = true;};

    void pauseThread();
    void resumeThread();

    bool isPrinterFree();
private:
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

    PrinterState m_state;
    std::queue<Coord::DecartPoint> m_printJob;

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

    PrinterPin pinRStep;
    PrinterPin pinRDir; // RESET - R>0
    PrinterPin pinRSensor;

    PrinterPin pinFiStep;
    PrinterPin pinFiDir;  // SET - clockwise direction
    PrinterPin pinFiSensor;

    bool fiCenterTrigger = false;
    bool rCenterTrigger = false;

    // HW-------------------------------------------
    void timersInit();
    void pinsInit();

    void setTIMPeriods(double_t rStepTime, double_t fiStepTime);

    uint32_t radiansToMotorTicks(double_t radians);
    uint32_t lengthToMotorTicks(double_t length);

    void setStep(double_t dR, double_t dFi, double_t stepTime);
    void stop();
    // HW----------------------------------------------------------

    static constexpr uint32_t timerPrescaler = 1000000; // 1us
    static constexpr uint16_t minRPeriod = 100;
    static constexpr uint16_t minFiPeriod = 250;

    static constexpr double_t printScaleCoef = 0.7;

    static constexpr double_t speed = 15;//7.5;
    static constexpr double_t stepSize = 1;

    static constexpr uint16_t rMoveDiapason = 270;

    static constexpr uint16_t uTicks = 16;
    static constexpr uint16_t motorRoundTicks = 200;

    static constexpr uint16_t rGearTeethCount = 20;
    static constexpr uint16_t rGearStep = 2;
    static constexpr double_t rTicksCoef = (double_t)uTicks * (double_t)motorRoundTicks / (double_t)(rGearStep * rGearTeethCount);

    static constexpr uint16_t fiGear1TeethCount = 20;
    static constexpr uint16_t fiGear2TeethCount = 160;//202;
    static constexpr double_t fiTicksCoef = (double_t)uTicks * (((double_t)fiGear2TeethCount/(double_t)fiGear1TeethCount) * motorRoundTicks) / (2 * M_PI);

    static constexpr double_t errRonRadian = fiGear1TeethCount*rGearStep/(2*M_PI); // 40mm error in R on 2pi (360) rotation
                                                // clockwise direction: -R

    static constexpr double_t mmOnRTick = 1/rTicksCoef;
    static constexpr double_t radOnFiTick = 1/fiTicksCoef;
    static constexpr double_t errRonTick = errRonRadian/fiTicksCoef;
};

#endif /* USER_PRINTER_HPP_ */
