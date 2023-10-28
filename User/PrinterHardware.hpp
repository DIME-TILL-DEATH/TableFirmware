#ifndef USER_PRINTERHARDWARE_HPP_
#define USER_PRINTERHARDWARE_HPP_

#define EXTI_RSENS_LINE EXTI_Line10
#define EXTI_FISENS_LINE EXTI_Line13

#include <vector>
#include <queue>

#include "debug.h"

#include "Coordinates.hpp"


typedef struct
{
    GPIO_TypeDef* port;
    uint16_t pin;
}PrinterPin;

class PrinterHardware
{
public:
    PrinterHardware();

    void pause();
    void resume();
    void stop();
private:
    typedef enum
    {
        ERROR = 0,
        IDLE,
        PRINTING,
        PAUSE,
        SET_STEP,
        SEARCH_R_ZERO,
        SEARCH_FI_ZERO,
        CORRECTING_CENTER
    }PrinterHWState;

    PrinterHWState m_state, m_prevState;

    Coord::DecartPoint currentPosition{0, 0};
    Coord::DecartPoint targetPosition{0, 0};

    Coord::PolarPoint currentPolarPosition{0, 0};
    Coord::PolarPoint targetPolarPosition{0, 0};

    uint32_t rTicksCounter{0};
    uint32_t fiTicksCounter{0};

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

    void timersInit();
    void pinsInit();

//    void setTIMPeriods(double_t rStepTime, double_t fiStepTime);
//
//    uint32_t radiansToMotorTicks(double_t radians);
//    uint32_t lengthToMotorTicks(double_t length);
//
//    void setStep(double_t dR, double_t dFi, double_t stepTime);

    //===================CONSTANTS==========================
    static constexpr uint32_t timerPrescaler = 1000000; // 1us
    static constexpr uint16_t minRPeriod = 100;
    static constexpr uint16_t minFiPeriod = 250;

    static constexpr double_t printScaleCoef = 0.7;

    static constexpr double_t speed = 7.5;
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


#endif /* USER_PRINTERHARDWARE_HPP_ */
