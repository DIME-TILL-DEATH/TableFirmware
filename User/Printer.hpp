#ifndef USER_PRINTER_HPP_
#define USER_PRINTER_HPP_

#include <vector>
#include <queue>

#include "debug.h"

#include "Coordinates.hpp"

typedef enum
{
    ERROR = 0,
    IDLE,
    SET_POINT,
    PRINTING,
    SET_STEP
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

    PrinterState state();

    bool findCenter();

    void pushPrintPoint(const Coord::DecartPoint& printPoint);
    void printRoutine();

    void makeRStep();
    void makeFiStep();
private:
    PrinterState m_state;
    std::queue<Coord::DecartPoint> m_printJob;

    Coord::DecartPoint currentPosition{0, 0};
    Coord::DecartPoint targetPosition{0, 0};

    uint32_t rTicksCounter{0};
    uint32_t fiTicksCounter{0};

    uint32_t fiSumTicks{0};

    double_t stepX;
    double_t stepY;
    double_t stepTime;

    PrinterPin pinRStep;
    PrinterPin pinRDir;
    PrinterPin pinFiStep;
    PrinterPin pinFiDir;

    void timersInit();
    void pinsInit();

    void setRStepPeriod(double_t timeInSec);
    void setFiStepPeriod(double_t timeInSec);

    uint32_t radiansToMotorTicks(double_t radians);
    uint32_t lengthToMotorTicks(double_t length);

    static constexpr uint32_t timerPrescaler = 1000000; // 1us

    static constexpr double_t speed = 500; //
    static constexpr double_t stepSize = 0.5; // step size = 1mm

    static constexpr uint16_t uTicks = 16;
    static constexpr uint16_t motorRoundTicks = 200;

    double_t rTicksCoef;
    static constexpr uint16_t rGearTeethCount = 20;
    static constexpr uint16_t rGearStep = 2;

    double_t fiTicksCoef;
    static constexpr uint16_t fiGear1TeethCount = 20;
    static constexpr uint16_t fiGear2TeethCount = 20;//202;

};

#endif /* USER_PRINTER_HPP_ */
