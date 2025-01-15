#include "abstractledstrip.hpp"


AbstractLedStrip::AbstractLedStrip()
{

}

void AbstractLedStrip::setBrightness(float newBrightness)
{
    brightness = newBrightness;

}

float AbstractLedStrip::getBrightness()
{
    return brightness;
}