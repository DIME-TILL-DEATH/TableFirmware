#include "coordinates.hpp"

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

bool Coord::isLinesCross(DecartPoint point11, DecartPoint point12, DecartPoint point21, DecartPoint point22)
{
    double_t x1 = point11.x;
    double_t x2 = point12.x;
    double_t x3 = point21.x;
    double_t x4 = point22.x;

    double_t y1 = point11.y;
    double_t y2 = point12.y;
    double_t y3 = point21.y;
    double_t y4 = point22.y;

    double_t denominator=(y4-y3)*(x1-x2)-(x4-x3)*(y1-y2);

    if (denominator == 0)
    {
        if ( (x1*y2-x2*y1)*(x4-x3) - (x3*y4-x4*y3)*(x2-x1) == 0 && (x1*y2-x2*y1)*(y4-y3) - (x3*y4-x4*y3)*(y2-y1) == 0)
            return true;
        else
            return false;
    }
    else
    {
        double_t numerator_a=(x4-x2)*(y4-y3)-(x4-x3)*(y4-y2);
        double_t numerator_b=(x1-x2)*(y4-y2)-(x4-x2)*(y1-y2);
        double_t Ua=numerator_a/denominator;
        double_t Ub=numerator_b/denominator;

        if (Ua >=0 && Ua <=1 && Ub >=0 && Ub <=1)  return true;
        else return false;
    }
}
