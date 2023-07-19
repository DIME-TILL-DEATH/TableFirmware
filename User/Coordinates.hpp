#ifndef USER_COORDINATES_HPP_
#define USER_COORDINATES_HPP_

#include "debug.h"
#include <math.h>

namespace Coord
{

typedef struct
{
    float_t x;
    float_t y;
}DecartPoint;


typedef struct
{
    float_t fi;
    float_t r;
}PolarPoint;

PolarPoint convertDecartToPolar(const DecartPoint& decartPoint);
DecartPoint convertPolarToDecart(const PolarPoint& polarPoint);
};


#endif /* USER_COORDINATES_HPP_ */
