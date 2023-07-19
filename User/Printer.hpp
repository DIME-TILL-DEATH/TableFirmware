#ifndef USER_PRINTER_HPP_
#define USER_PRINTER_HPP_

#include "debug.h"

#include "Coordinates.hpp"
#include <vector>
#include <queue>

typedef enum
{
    IDLE = 0,
    PRINTING,
    ERROR,
    WAIT,
    FINISHED
}PrinterState;

class Printer
{
public:
    Printer();

    PrinterState state();

    void pushPrintBlock(const std::vector<Coord::PolarPoint>& printBlock);
    void printNextPoint();
private:
    PrinterState m_state;
    std::queue<Coord::PolarPoint> m_printJob;
};

#endif /* USER_PRINTER_HPP_ */
