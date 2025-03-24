#ifndef USER_POLARPRINTER_HPP_
#define USER_POLARPRINTER_HPP_

#include "abstractprinter.hpp"

class PolarPrinter : public AbstractPrinter
{
public:
    PolarPrinter();

    void loadSettings();
    void findCenter();
    void printRoutine();

    void firtsMotorMakeStep();
    void secondMotorMakeStep();

    void setFiGearTeethCount(uint16_t newFiGear1TeethCount, uint16_t newFiGear2TeethCount);
    uint16_t getFiGear2TeethsCount() {return fiGear2TeethCount;};

    void setRGearTeethCount(uint16_t newRGearTeethCount);
private:
    Coord::PolarPoint currentPolarPosition{0, 0};
    Coord::PolarPoint targetPolarPosition{0, 0};

    float_t coordSysRotation;
    float_t correctionLength{0};

    uint16_t rMoveDiapason = 500;

    uint16_t rGearTeethCount = 20;
    uint16_t rGearStep = 2;
    float_t rTicksCoef; // = (float_t)uTicks * (float_t)motorRoundTicks / (float_t)(rGearStep * rGearTeethCount);
    
    uint16_t fiGear1TeethCount;// = 20;
    uint16_t fiGear2TeethCount;// = 160;//202;
    float_t fiTicksCoef; // = (float_t)uTicks * (((float_t)fiGear2TeethCount/(float_t)fiGear1TeethCount) * motorRoundTicks) / (2 * M_PI);
    float_t errRonRadian; // = fiGear1TeethCount*rGearStep/(2*M_PI); // 40mm error in R on 2pi (360) rotation
                                                // clockwise direction: -R
    float_t mmOnRTick;// = 1/rTicksCoef;
    float_t radOnFiTick;// = 1/fiTicksCoef;
    float_t errRonTick;// = errRonRadian/fiTicksCoef;

    void setStep(double_t dR, double_t dFi, double_t stepTime);

    uint32_t radiansToMotorTicks(double_t radians);
    uint32_t lengthToMotorTicks(double_t length);

    void recountValuesOnTick();
};

#endif