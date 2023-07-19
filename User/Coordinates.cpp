#include "Coordinates.hpp"

using namespace Coord;

PolarPoint Coord::convertDecartToPolar(const DecartPoint& decartPoint)
{
    PolarPoint polarPoint;

    polarPoint.r = pow(decartPoint.x*decartPoint.x + decartPoint.y*decartPoint.y, 0.5);
    polarPoint.fi = tan(decartPoint.x/decartPoint.y);

    return polarPoint;
}

DecartPoint Coord::convertPolarToDecart(const PolarPoint& polarPoint)
{
    DecartPoint decartPoint;

    decartPoint.x = polarPoint.r*cos(polarPoint.fi);
    decartPoint.y = polarPoint.r*sin(polarPoint.fi);

    return decartPoint;
}
