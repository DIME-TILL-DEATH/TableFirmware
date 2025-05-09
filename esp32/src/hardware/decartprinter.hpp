#ifndef USER_DECARTPRINTER_HPP_
#define USER_DECARTPRINTER_HPP_

#include "abstractprinter.hpp"

class DecartPrinter : public AbstractPrinter
{
public:
    DecartPrinter();

    void loadSettings();
    void findCenter();
    void printRoutine();

    void firtsMotorMakeStep();
    void secondMotorMakeStep();
private:
    uint16_t xMoveDiapason = 740;
    uint16_t yMoveDiapason = 460;

    double lengthTicksCoef;

    uint16_t gearTeethCount = 20;
    uint16_t gearStep = 2;

    void setMove(double_t dX, double_t dY, double_t moveTime);

    int32_t lengthToMotorTicks(double_t length);
};

#endif