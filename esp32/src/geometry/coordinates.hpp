#ifndef COORDINATES_HPP
#define COORDINATES_HPP

#include <math.h>

namespace Coord
{

typedef struct
{
    double_t x;
    double_t y;
}DecartPoint;

typedef struct
{
    double_t fi;
    double_t r;
}PolarPoint;

PolarPoint convertDecartToPolar(const DecartPoint& decartPoint);
DecartPoint convertPolarToDecart(const PolarPoint& polarPoint);

bool isLinesCross(DecartPoint point11, DecartPoint point12, DecartPoint point21, DecartPoint point22);
};


#endif /* COORDINATES_HPP */
