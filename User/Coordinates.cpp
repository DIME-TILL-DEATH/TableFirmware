#include "Coordinates.hpp"

using namespace Coord;

PolarPoint Coord::convertDecartToPolar(const DecartPoint& decartPoint)
{
    PolarPoint polarPoint{0, 0};

    polarPoint.r = sqrt(decartPoint.x*decartPoint.x + decartPoint.y*decartPoint.y);

    if(decartPoint.x != 0)
    {
        polarPoint.fi = atan(decartPoint.y/decartPoint.x);
    }
    else
    {
        if(decartPoint.y > 0) polarPoint.fi = M_PI_2;
        else polarPoint.fi = 3 * M_PI_2;
    }

    return polarPoint;
}

DecartPoint Coord::convertPolarToDecart(const PolarPoint& polarPoint)
{
    DecartPoint decartPoint;

    decartPoint.x = polarPoint.r*cos(polarPoint.fi);
    decartPoint.y = polarPoint.r*sin(polarPoint.fi);

    return decartPoint;
}
