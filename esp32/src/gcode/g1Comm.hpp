#ifndef G1COMM_HPP
#define G1COMM_HPP

#include "gabstractcomm.hpp"
#include "geometry/coordinates.hpp"

namespace GCode
{

class G1Comm : public GAbstractComm
{
public:
    G1Comm(Coord::DecartPoint targetPoint, float_t targetSpeed);

    Coord::DecartPoint decartCoordinates();
    Coord::PolarPoint polarCoordinates();
    float_t speed();
private:
    Coord::DecartPoint m_targetDecartPoint;
    float_t m_targetSpeed;
};


};

#endif /* G1COMM_HPP */
