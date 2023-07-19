#include "G1Comm.hpp"

using namespace GCode;


G1Comm::G1Comm(Coord::DecartPoint targetPoint, float_t targetSpeed)
{
    m_commType = GCommType::G1;

    m_targetDecartPoint = targetPoint;
    m_targetSpeed = targetSpeed;
}


Coord::DecartPoint G1Comm::decartCoordinates()
{
    return m_targetDecartPoint;
}

Coord::PolarPoint G1Comm::polarCoordinates()
{
    return Coord::convertDecartToPolar(m_targetDecartPoint);
}

float_t G1Comm::speed()
{
    return m_targetSpeed;
}
