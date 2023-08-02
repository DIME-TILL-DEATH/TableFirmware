#include "Coordinates.hpp"

using namespace Coord;

PolarPoint Coord::convertDecartToPolar(const DecartPoint& decartPoint)
{
    PolarPoint polarPoint{0, 0};

    polarPoint.r = sqrt(decartPoint.x*decartPoint.x + decartPoint.y*decartPoint.y);
    polarPoint.fi = atan2(decartPoint.y, decartPoint.x);

    if(decartPoint.y < 0) polarPoint.fi += M_PI * 2;

    return polarPoint;
}

DecartPoint Coord::convertPolarToDecart(const PolarPoint& polarPoint)
{
    DecartPoint decartPoint;

    decartPoint.x = polarPoint.r*cos(polarPoint.fi);
    decartPoint.y = polarPoint.r*sin(polarPoint.fi);

    return decartPoint;
}
