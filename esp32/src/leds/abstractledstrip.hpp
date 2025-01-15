#ifndef ABSTRACTLEDSTRIP_HPP
#define ABSTRACTLEDSTRIP_HPP

class AbstractLedStrip
{
public:

    AbstractLedStrip();

    virtual void setBrightness(float newBrightness);
    float getBrightness();

protected:
    float brightness{0.5};

    constexpr static char TAG[] = "LED STRIP";
};

#endif