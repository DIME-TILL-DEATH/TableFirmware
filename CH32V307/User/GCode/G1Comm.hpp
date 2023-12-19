#ifndef USER_GCODE_G1COMM_HPP_
#define USER_GCODE_G1COMM_HPP_

#include "GAbstractComm.hpp"
#include "Coordinates.hpp"

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

#endif /* USER_GCODE_G1COMM_HPP_ */
