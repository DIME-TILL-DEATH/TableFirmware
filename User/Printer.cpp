#include "Printer.hpp"

Printer::Printer()
{
    m_state = PrinterState::IDLE;
}

PrinterState Printer::state()
{
    return m_state;
}

void Printer::pushPrintBlock(const std::vector<Coord::PolarPoint>& printBlock)
{
    for(auto coord_it = printBlock.begin(); coord_it != printBlock.end(); ++coord_it)
    {
        m_printJob.push(*coord_it);
    }
}

void Printer::printNextPoint()
{
    Coord::PolarPoint nextPoint;

    if(m_printJob.size()>0)
    {
        nextPoint = m_printJob.front();

        printf("====next point r=%f pi=%f", nextPoint.r, nextPoint.fi);

        m_printJob.pop();
        m_state = PrinterState::PRINTING;
    }
    else
    {
//        switch(m_state)
//        {
//        case PrinterState::PRINTING: m_state = PrinterState::FINISHED;break;
//        }
        if(m_state == PrinterState::PRINTING) m_state = PrinterState::FINISHED;
    }
}
