#include "polarprinter.hpp"

#include <math.h>
#include "esp_log.h"

#include "filemanager/settings.hpp"

PolarPrinter::PolarPrinter()
{
    coordSysRotation = 0;

    currentPolarPosition.r = 0;
    currentPolarPosition.fi = 0;

    setRGearTeethCount(20);
    setFiGearTeethCount(20, 160);  

    minRPeriod = 250;
    minFiPeriod = 350;
}

void PolarPrinter::loadSettings()
{
    speed = Settings::getSetting(Settings::Digit::PRINT_SPEED);
    coordSysRotation = (Settings::getSetting(Settings::Digit::PRINT_ROTATION) / 180) * M_PI;
    printScaleCoef = Settings::getSetting(Settings::Digit::SCALE_COEF);
    correctionLength = Settings::getSetting(Settings::Digit::CORRETION_LENGTH);
    pauseInterval = Settings::getSetting(Settings::Digit::PAUSE_INTERVAL);

    uint16_t settingFiGear2TeethCount = Settings::getSetting(Settings::Digit::FI_GEAR2_TEETH_COUNT);
    setRGearTeethCount(20);
    setFiGearTeethCount(20, settingFiGear2TeethCount);

    rMoveDiapason = rMoveDiapason * printScaleCoef;

    ESP_LOGI(TAG, "Loading printer settings. Speed: %f, Rotation: %f, Scale: %f, Correction: %f, Pause: %d, FiGear2Teeths: %d", 
    speed, coordSysRotation, printScaleCoef, correctionLength, pauseInterval, settingFiGear2TeethCount);  
}

void PolarPrinter::findCenter()
{
    pauseThread();

    secondCoordTimer->setInterval(350);
    firstCoordTimer->setInterval(350);

    printf("=>Finding table center...\r\n");
    currentPosition.x = 0;
    currentPosition.y = 0;

    pointNum = 1;

    firstCoordEndstopTrigger = false;
    secondCoordEndstopTrigger = false;
    setState(PrinterState::SEARCH_FIRST_ZERO);

#ifdef GLOBAL_IGNORE_ENDSTOPS
    fiCenterTrigger = true;
#else
    if(gpio_get_level(PIN_FISRT_ENDSTOP) == Pins::PinState::SET)
    {
        setStep(0, M_PI/8, 2);
        setState(PrinterState::RESCAN_FIRST_ZERO);
    }
    else
    {
        setStep(0, -2*M_PI, 20);
    }
#endif  

#ifdef GLOBAL_IGNORE_ENDSTOPS
    rCenterTrigger = true;
#else
    if(gpio_get_level(PIN_SECOND_ENDSTOP) == Pins::PinState::RESET)
    { 
       secondCoordEndstopTrigger = true;
    }
#endif 

    gpio_intr_enable(PIN_SECOND_ENDSTOP);
    gpio_intr_enable(PIN_FISRT_ENDSTOP);
  
    resumeThread();
}

void PolarPrinter::printRoutine()
{
    switch(m_state)
    {
        case PrinterState::IDLE:
        {
            if(nextComm != nullptr)
            {
                setState(PrinterState::HANDLE_COMMAND);
                printRoutine();
            }
            break;
        }

        case PrinterState::HANDLE_COMMAND:
        {
            if(nextComm != nullptr)
            {
                switch(nextComm->commType())
                {
                    case GCode::GCommType::M51:
                    {
                        //findCenter();
                        pointNum = 1;
                        break;
                    }
                    case GCode::GCommType::G1:
                    {
                        gpio_set_level(GPIO_NUM_23, 1);
                        GCode::G1Comm* g1Comm = static_cast<GCode::G1Comm*>(nextComm);
                        if(g1Comm)
                        {
                            targetPosition = g1Comm->decartCoordinates();
                            targetPosition.x = targetPosition.x * printScaleCoef;
                            targetPosition.y = targetPosition.y * printScaleCoef;

                            setState(PrinterState::SET_POINT);
                            printRoutine();
                            //printf("point(%lf, %lf)", targetPosition.x, targetPosition.y);
                        }
                        break;
                    }
                    case GCode::GCommType::G4:
                    {
                        GCode::G4Comm* g4Comm = static_cast<GCode::G4Comm*>(nextComm);
                        if(g4Comm)
                        {
                            printf("End of file, pause inteval: %d.\r\n", pauseInterval);
                            pause(pauseInterval); //g4Comm->pause()
                        }
                        break;
                    }
                    default: ESP_LOGE("PRINTER", "Unhandled CGomm");
                }
                delete nextComm;
                nextComm = nullptr;
            }
            else
            {
                setState(PrinterState::IDLE);
            }
            break;
        }

        case PrinterState::SET_POINT:
        {
            if(targetPosition.x == currentPosition.x && currentPosition.y == targetPosition.y)
            {
                // setState(PrinterState::HANDLE_COMMAND);
                setState(PrinterState::IDLE);
                return; //skip point
            }

            float_t deltaX = targetPosition.x - currentPosition.x;
            float_t deltaY = targetPosition.y - currentPosition.y;
            targetPolarPosition = Coord::convertDecartToPolar(targetPosition);

            float_t lineLength = sqrt(pow((deltaX), 2) + pow((deltaY), 2));
            float_t steps = lineLength/stepSize;
            stepX = deltaX / steps;
            stepY = deltaY / steps;

            stepTime =  (sqrt(pow((stepX), 2) + pow((stepY), 2)));// / speed;

            // INFO block==========
            //    printf("Point num: %d\r\n", pointNum);


            //    printf("DECART: current(%lf, %lf), target(%lf, %lf)\r\n", currentPosition.x, currentPosition.y, targetPosition.x, targetPosition.y);
            //    printf("length: %lf, steps: %lf, stepX: %lf, stepY: %lf\r\n", lineLength, steps, stepX, stepY);
            //    printf("POLAR: current(%lf, %lf), target(%lf, %lf)\r\n", currentPolarPosition.r, currentPolarPosition.fi* 360 / (M_PI * 2), 
            //    targetPolarPosition.r, targetPolarPosition.fi* 360 / (M_PI * 2));
            //    printf("\r\n");
            //=================================

            pointNum++;

            setState(PrinterState::SET_STEP);
            gpio_set_level(GPIO_NUM_23, 0);
            printRoutine();
            break;
        }

        case PrinterState::SET_STEP:
        {
            double_t minErrX = (stepX == 0) ? stepSize : fabs(stepX);
            double_t minErrY = (stepY == 0) ? stepSize : fabs(stepY);

            if((fabs(targetPosition.x - currentPosition.x) < minErrX) && (fabs(targetPosition.y - currentPosition.y) < minErrY))
            {
                gpio_set_level(GPIO_NUM_23, 1);
                currentPosition = Coord::convertPolarToDecart(currentPolarPosition);
                // setState(PrinterState::HANDLE_COMMAND);
                setState(PrinterState::IDLE);
                gpio_set_level(GPIO_NUM_23, 0);
                // printRoutine();
            }
            else
            {
                Coord::DecartPoint stepPosition{currentPosition.x + stepX, currentPosition.y + stepY};

                Coord::PolarPoint curPolarPoint = Coord::convertDecartToPolar(currentPosition);
                Coord::PolarPoint stepPolarPoint = Coord::convertDecartToPolar(stepPosition);

                double_t deltaR = stepPolarPoint.r - curPolarPoint.r;
                double_t deltaFi = stepPolarPoint.fi - curPolarPoint.fi;

                // x-axis positive cross(polar +2pi) not align
                if(fabs(deltaFi) > M_PI)
                {
                    if(Coord::isLinesCross(currentPosition, stepPosition, {0, 0}, {1000, 0}))
                    {
                        deltaFi = (-1) * copysign(2 * M_PI - fabs(deltaFi), deltaFi);
                    }
                }

                setStep(deltaR, deltaFi, stepTime/speed);

                //printf("current fi: %lf, delta fi: %lf\r\n", currentPolarPosition.fi* 360 / (M_PI * 2), deltaFi * 360 / (M_PI * 2));

                currentPosition.x += stepX;
                currentPosition.y += stepY;

                // Avoid overflow on circular printing in one direction
                if(currentPolarPosition.fi < 0)
                {
                    currentPolarPosition.fi += 2 * M_PI;
                }
                if(currentPolarPosition.fi > 2 * M_PI)
                {
                    currentPolarPosition.fi -= 2 * M_PI;
                }

                setState(PrinterState::PRINTING);
            }
            break;
        }

        case PrinterState::PRINTING:
        {
            if(fabs(targetPolarPosition.r-currentPolarPosition.r) < 0.25 && fabs(targetPolarPosition.fi-currentPolarPosition.fi) < 0.5 * 2* M_PI/360)
            {
                gpio_set_level(GPIO_NUM_23, 1);
                //printf("===coords compare finish\r\n");
                secondMotorTicksCounter = 0;
                firstMotorTicksCounter = 0;
                currentPosition = Coord::convertPolarToDecart(currentPolarPosition);
                // setState(PrinterState::HANDLE_COMMAND);
                setState(PrinterState::IDLE);
                gpio_set_level(GPIO_NUM_23, 0);
                // printRoutine();  // kill pause  
                break;
            }

            if(secondMotorTicksCounter==0 && firstMotorTicksCounter==0)
            {
                setState(PrinterState::SET_STEP);   
                printRoutine();  // kill pause  
            }
            break;
        }

        case PrinterState::RESCAN_FIRST_ZERO:
        {
            if(firstMotorTicksCounter == 0)
            {
                setStep(0, -2*M_PI, 25);
                setState(PrinterState::SEARCH_FIRST_ZERO);
                printf("Fi rescaning\r\n");
                firstCoordEndstopTrigger = false;
            }
            break;
        }

        case PrinterState::SEARCH_FIRST_ZERO:
        {
            if(firstCoordEndstopTrigger)
            {               
                setStep(-rMoveDiapason * 1.1, 0, 7.5); // 10% margin
                setState(PrinterState::SEARCH_SECOND_ZERO);
                printf("Fi zeroed\r\n");
            }
            break;
        }

        case PrinterState::RESCAN_SECOND_ZERO:
        {
            if(secondMotorTicksCounter == 0)
            {
                setStep(0, M_PI/4, 2.5); 
                setState(PrinterState::RESCAN_FIRST_ZERO);
                secondCoordEndstopTrigger = false;
                firstCoordEndstopTrigger = false;
                printf("R rescaning\r\n");
            }
            break;
        }

        case PrinterState::SEARCH_SECOND_ZERO:
        {
            if(secondCoordEndstopTrigger)
            {
                abortPoint();

                gpio_intr_disable(PIN_SECOND_ENDSTOP);
                gpio_intr_disable(PIN_FISRT_ENDSTOP);

                printf("R zeroed\r\n");

                setStep(correctionLength, 0, 2);
                setState(PrinterState::CORRECTING_CENTER);
                break;
            }
            else if(secondMotorTicksCounter == 0)
            {
                ESP_LOGE(TAG, "R center nor found! Pad is out of bounds. Correcting\r\n");
                setStep(rMoveDiapason * 1.5, 0, 2 * rMoveDiapason / 370);
                setState(PrinterState::RESCAN_SECOND_ZERO);
            }
            break;
        }

        case PrinterState::CORRECTING_CENTER:
        {
            if(secondMotorTicksCounter == 0)
            {
                currentPolarPosition.r = 0;
                currentPolarPosition.fi = 0;

                setStep(0, coordSysRotation, 12.5 * coordSysRotation / M_PI );
                setState(PrinterState::ROTATE_COORD_SYS);
                printf("Center corrected\r\n");
            }
            break;
        }

        case PrinterState::ROTATE_COORD_SYS:
        {
            if(firstMotorTicksCounter == 0)
            {
                currentPolarPosition.r = 0;
                currentPolarPosition.fi = 0;
                setState(PrinterState::IDLE);
                printf("Coord sys rotated\r\n");
            }
            break;
        }

        case PrinterState::PAUSE:
        {
            if(pauseCounter != infinitePause)
            {
                if(pauseCounter>0)
                {
                    pauseCounter--;
                }
                else
                {
                    returnToPreviousState();
                }
            }
            break;
        }

        default: {}
    }
}

void PolarPrinter::setStep(double_t dR, double_t dFi, double_t stepTimeInSec)
{
    bool rDirection;

    secondMotorTicksCounter = lengthToMotorTicks(abs(dR));
    int32_t correctionTicks = lengthToMotorTicks(abs(dFi) * errRonRadian);

    if(dFi > 0) // counter clokwise, R decrease
    {
        printerPins->setFirstMotorDirState(Pins::PinState::RESET);
        if(dR >= 0)
        {
            rDirection = 0;
            secondMotorTicksCounter += correctionTicks;
        }
        else
        {
            int32_t resultTicks = secondMotorTicksCounter - correctionTicks;

            if(resultTicks > 0) rDirection = 1;
            else rDirection = 0;

            secondMotorTicksCounter = abs(resultTicks);
        }
    }
    else // clockwise, R increase
    {
        printerPins->setFirstMotorDirState(Pins::PinState::SET);
        if(dR >= 0)
        {
            int32_t resultTicks = secondMotorTicksCounter - correctionTicks;

            if(resultTicks > 0) rDirection = 0;
            else rDirection = 1;

            secondMotorTicksCounter = abs(resultTicks);
        }
        else
        {
            rDirection = 1;
            secondMotorTicksCounter += correctionTicks;
        }
    }

    printerPins->setSecondMotorDirState(static_cast<Pins::PinState>(rDirection));

    firstMotorTicksCounter = radiansToMotorTicks(abs(dFi));
    //if(abs(dFi) > M_PI_2) printf("Long dFi move, dFi: %lf, fi ticks: %d\r\n", dFi, fiTicksCounter);


    float_t rPeriod = stepTimeInSec/secondMotorTicksCounter;
    float_t fiPeriod = stepTimeInSec/firstMotorTicksCounter;

    setTIMPeriods(fiPeriod, rPeriod);
}


uint32_t PolarPrinter::radiansToMotorTicks(double_t radians)
{
    return round(fiTicksCoef * radians);
}

uint32_t PolarPrinter::lengthToMotorTicks(double_t length)
{
    return round(rTicksCoef * length);
}

void PolarPrinter::setFiGearTeethCount(uint16_t newFiGear1TeethCount, uint16_t newFiGear2TeethCount)
{
    fiGear1TeethCount = newFiGear1TeethCount;
    fiGear2TeethCount = newFiGear2TeethCount;

    fiTicksCoef = (float_t)uTicks * (((float_t)fiGear2TeethCount/(float_t)fiGear1TeethCount) * motorRoundTicks) / (2 * M_PI);
    errRonRadian = fiGear1TeethCount*rGearStep/(2*M_PI); // 40mm error in R on 2pi (360) rotation
                                                // clockwise direction: -R

    recountValuesOnTick();
}

void PolarPrinter::setRGearTeethCount(uint16_t newRGearTeethCount)
{
    rGearTeethCount = newRGearTeethCount;
    rTicksCoef = (float_t)uTicks * (float_t)motorRoundTicks / (float_t)(rGearStep * rGearTeethCount);
    recountValuesOnTick();
}

void PolarPrinter::recountValuesOnTick()
{
    mmOnRTick = 1/rTicksCoef;
    radOnFiTick = 1/fiTicksCoef;
    errRonTick = errRonRadian/fiTicksCoef;    
}

void IRAM_ATTR PolarPrinter::firtsMotorMakeStep()
{
    if(m_state == PrinterState::PRINTING || 
    m_state == PrinterState::SEARCH_FIRST_ZERO || m_state == PrinterState::SEARCH_SECOND_ZERO || 
    m_state == PrinterState::CORRECTING_CENTER || m_state == PrinterState::ROTATE_COORD_SYS ||
    m_state == PrinterState::RESCAN_FIRST_ZERO || m_state == PrinterState::RESCAN_SECOND_ZERO)
    {
        if(firstMotorTicksCounter>0)
        {
            if(printerPins->getFirstMotorStep() == Pins::PinState::RESET)
            {
                printerPins->setFirstMotorStepState(Pins::PinState::SET);

                if(printerPins->getFirstMotorDir() == Pins::PinState::RESET)
                {
                    currentPolarPosition.fi += radOnFiTick;
                    currentPolarPosition.r -= errRonTick;
                }
                else
                {
                    currentPolarPosition.fi -= radOnFiTick;
                    currentPolarPosition.r += errRonTick;
                }
            }
            else
            {
                printerPins->setFirstMotorStepState(Pins::PinState::RESET);
                firstMotorTicksCounter--;
            }
        }
    }
}

void IRAM_ATTR PolarPrinter::secondMotorMakeStep()
{
    if(m_state == PrinterState::PRINTING || 
    m_state == PrinterState::SEARCH_FIRST_ZERO || m_state == PrinterState::SEARCH_SECOND_ZERO || 
    m_state == PrinterState::CORRECTING_CENTER || m_state == PrinterState::ROTATE_COORD_SYS ||
    m_state == PrinterState::RESCAN_FIRST_ZERO || m_state == PrinterState::RESCAN_SECOND_ZERO)
    {
        if(secondMotorTicksCounter>0)
        {
            if(printerPins->getSecondMotorStep() == Pins::PinState::RESET)
            {

                printerPins->setSecondMotorStepState(Pins::PinState::SET);

                if(printerPins->getSecondMotorDir() == Pins::PinState::RESET)
                {
                    currentPolarPosition.r += mmOnRTick;
                }
                else
                {
                    currentPolarPosition.r -= mmOnRTick;
                }
            }
            else
            {
                printerPins->setSecondMotorStepState(Pins::PinState::RESET);
                secondMotorTicksCounter--;
            }
        }
    }
}