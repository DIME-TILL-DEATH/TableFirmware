#ifndef USER_PRINTER_HPP_
#define USER_PRINTER_HPP_

#include "debug.h"

#include "Coordinates.hpp"
#include <vector>
#include <queue>

typedef enum
{
    ERROR = 0,
    IDLE,
    SETTLING,
    PRINTING,
    FINISHED_STEP
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
    Coord::DecartPoint nextPoint;

    uint32_t rTicksCounter{0};
    uint32_t fiTicksCounter{0};

    PrinterPin pinRStep;
    PrinterPin pinRDir;
    PrinterPin pinFiStep;
    PrinterPin pinFiDir;

    void timersInit();
    void pinsInit();

    void setRStepPeriod(float_t timeInSec);
    void setFiStepPeriod(float_t timeInSec);

    uint32_t radiansToMotorTicks(float_t radians);

    static constexpr uint32_t timerPrescaler = 1000000; // 1us

    static constexpr float_t speed = 10; // mm/sec

    static constexpr uint16_t uTicks = 16;

    static constexpr uint16_t stepSize = 1; // step size = 1mm

    static constexpr uint16_t rTicksCoef = 1;

    float_t fiTicksCoef;
    static constexpr uint16_t gear1TeethCount = 20;
    static constexpr uint16_t gear2TeethCount = 202;
    static constexpr uint16_t motorRoundTicks = 200;
};

#endif /* USER_PRINTER_HPP_ */
