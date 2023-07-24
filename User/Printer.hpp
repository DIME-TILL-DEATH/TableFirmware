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

    uint32_t targetSteps;

    uint32_t rStepTicks{0};
    uint32_t rTicksCounter{0};

    uint32_t fiStepTicks{0};
    uint32_t fiTicksCounter{0};

    PrinterPin pinRStep;
    PrinterPin pinRDir;
    PrinterPin pinFiStep;
    PrinterPin pinFiDir;

    void timersInit();
    void pinsInit();

    static constexpr uint16_t stepSize = 10; // step size = 1mm

    static constexpr uint16_t rTicksCoef = 1000;
    static constexpr uint16_t fiTicksCoef = 10000;
};

#endif /* USER_PRINTER_HPP_ */
