#include "decartprinter.hpp"

#include <math.h>
#include "esp_log.h"

#include "filemanager/settings.hpp"

DecartPrinter::DecartPrinter()
{
    minRPeriod = 200;
    minFiPeriod = 200;

    lengthTicksCoef = (float_t)uTicks * (float_t)motorRoundTicks / (float_t)(gearStep * gearTeethCount);

    ESP_LOGI(TAG, "Decart printer created");
}

void DecartPrinter::loadSettings()
{
    speed = Settings::getSetting(Settings::Digit::PRINT_SPEED);
    printScaleCoef = Settings::getSetting(Settings::Digit::SCALE_COEF);
    pauseInterval = Settings::getSetting(Settings::Digit::PAUSE_INTERVAL);

    // uint16_t settingFiGear2TeethCount = Settings::getSetting(Settings::Digit::FI_GEAR2_TEETH_COUNT);
    // setRGearTeethCount(20);
    // setFiGearTeethCount(20, settingFiGear2TeethCount);

    xMoveDiapason = xMoveDiapason * printScaleCoef;
    yMoveDiapason = yMoveDiapason * printScaleCoef;

    ESP_LOGI(TAG, "Loading printer settings. Speed: %f, Scale: %f, Pause: %d", 
    speed, printScaleCoef, pauseInterval);  
}

void DecartPrinter::findCenter()
{
    pauseThread();

    secondCoordTimer->setInterval(200);
    firstCoordTimer->setInterval(200);

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
        setMove(xMoveDiapason / 10 / printScaleCoef, 0, 2);
        setState(PrinterState::RESCAN_FIRST_ZERO);
    }
    else
    {
        setMove(-xMoveDiapason * 1.1 / printScaleCoef, 0, 10);
    }
#endif  

#ifdef GLOBAL_IGNORE_ENDSTOPS
    rCenterTrigger = true;
#else
    if(gpio_get_level(PIN_SECOND_ENDSTOP) == Pins::PinState::SET)
    { 
       secondCoordEndstopTrigger = true;
    }
#endif 

    gpio_intr_enable(PIN_SECOND_ENDSTOP);
    gpio_intr_enable(PIN_FISRT_ENDSTOP);
  
    resumeThread();
}

void DecartPrinter::printRoutine()
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

            float_t lineLength = sqrt(pow((deltaX), 2) + pow((deltaY), 2));
            float_t steps = lineLength/stepSize;
            stepX = deltaX;// / steps;
            stepY = deltaY;// / steps;

            stepTime =  (sqrt(pow((stepX), 2) + pow((stepY), 2)));// / speed;

            // INFO block==========
               printf("Point num: %d\r\n", pointNum);


               printf("DECART: current(%lf, %lf), target(%lf, %lf)\r\n", currentPosition.x, currentPosition.y, targetPosition.x, targetPosition.y);
               printf("length: %lf, steps: %lf, stepX: %lf, stepY: %lf\r\n", lineLength, steps, stepX, stepY);
               printf("\r\n");
            //=================================

            pointNum++;

            setState(PrinterState::SET_STEP);
            gpio_set_level(GPIO_NUM_23, 0);
            printRoutine();
            break;
        }

        case PrinterState::SET_STEP:
        {
            // double_t minErrX = (stepX == 0) ? stepSize : fabs(stepX);
            // double_t minErrY = (stepY == 0) ? stepSize : fabs(stepY);

            // if((fabs(targetPosition.x - currentPosition.x) < minErrX) && (fabs(targetPosition.y - currentPosition.y) < minErrY))
            // {
            //     gpio_set_level(GPIO_NUM_23, 1);
            //     currentPosition = Coord::convertPolarToDecart(currentPolarPosition);
            //     // setState(PrinterState::HANDLE_COMMAND);
            //     setState(PrinterState::IDLE);
            //     gpio_set_level(GPIO_NUM_23, 0);
            //     // printRoutine();
            // }
            // else
            // {
                // Coord::DecartPoint stepPosition{currentPosition.x + stepX, currentPosition.y + stepY};

                // Coord::PolarPoint curPolarPoint = Coord::convertDecartToPolar(currentPosition);
                // Coord::PolarPoint stepPolarPoint = Coord::convertDecartToPolar(stepPosition);

                // double_t deltaX = stepPolarPoint.r - currentPosition.x;
                // double_t deltaFi = stepPolarPoint.fi - curPolarPoint.fi;

                setMove(stepX, stepY, stepTime/speed);

                //printf("current fi: %lf, delta fi: %lf\r\n", currentPolarPosition.fi* 360 / (M_PI * 2), deltaFi * 360 / (M_PI * 2));

                // currentPosition.x += stepX;
                // currentPosition.y += stepY;


                setState(PrinterState::PRINTING);
            // }
            break;
        }

        case PrinterState::PRINTING:
        {
            // if(fabs(targetPolarPosition.r-currentPolarPosition.r) < 0.25 && fabs(targetPolarPosition.fi-currentPolarPosition.fi) < 0.5 * 2* M_PI/360)
            // {
            //     gpio_set_level(GPIO_NUM_23, 1);
            //     //printf("===coords compare finish\r\n");
            //     secondMotorTicksCounter = 0;
            //     firstMotorTicksCounter = 0;
            //     currentPosition = Coord::convertPolarToDecart(currentPolarPosition);
            //     // setState(PrinterState::HANDLE_COMMAND);
            //     setState(PrinterState::IDLE);
            //     gpio_set_level(GPIO_NUM_23, 0);
            //     // printRoutine();  // kill pause  
            //     break;
            // }

            if(secondMotorTicksCounter==0 && firstMotorTicksCounter==0)
            {
                currentPosition.x += stepX;
                currentPosition.y += stepY;

                setState(PrinterState::SET_POINT);   
                printRoutine();  // kill pause  
            }
            break;
        }

        case PrinterState::RESCAN_FIRST_ZERO:
        {
            if(firstMotorTicksCounter == 0)
            {
                setMove(-xMoveDiapason * 1.1 / printScaleCoef, 0, 10);
                setState(PrinterState::SEARCH_FIRST_ZERO);
                printf("X rescaning\r\n");
                firstCoordEndstopTrigger = false;
            }
            break;
        }

        case PrinterState::SEARCH_FIRST_ZERO:
        {
            if(firstCoordEndstopTrigger)
            {               
                setMove(0, -yMoveDiapason * 1.1 / printScaleCoef, 10); // 10% margin
                setState(PrinterState::SEARCH_SECOND_ZERO);
                printf("X zeroed\r\n");
            }
            break;
        }

        case PrinterState::RESCAN_SECOND_ZERO:
        {
            if(secondMotorTicksCounter == 0)
            {
                setMove(xMoveDiapason/10/printScaleCoef, 0, 2); 
                setState(PrinterState::RESCAN_FIRST_ZERO);
                secondCoordEndstopTrigger = false;
                firstCoordEndstopTrigger = false;
                printf("Y rescaning\r\n");
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

                printf("Y zeroed\r\n");

                setMove(xMoveDiapason/2/printScaleCoef, yMoveDiapason/2/printScaleCoef - 5, 5);

                setState(PrinterState::CORRECTING_CENTER);
                break;
            }
            else if(secondMotorTicksCounter == 0)
            {
                ESP_LOGE(TAG, "Y edge not found! Pad is out of bounds.\r\n");
                setMove(0, yMoveDiapason/10, 10);
                setState(PrinterState::RESCAN_SECOND_ZERO);
            }
            break;
        }

        case PrinterState::CORRECTING_CENTER:
        {
            if(secondMotorTicksCounter == 0)
            {
                currentPosition.x = 0;
                currentPosition.y = 0;

                setState(PrinterState::IDLE);
                printf("Center settled\r\n");
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

        default: 
        {
            setState(PrinterState::IDLE);
        }
    }
}

void DecartPrinter::setMove(double_t dX, double_t dY, double_t moveTimeInSec)
{

//  printf("Long dFi move, dFi: %lf, fi ticks: %d\r\n", dFi, fiTicksCounter);

    int32_t motor1Ticks_sign = lengthToMotorTicks(dX) + lengthToMotorTicks(-dY);
    int32_t motor2Ticks_sign = lengthToMotorTicks(dX) - lengthToMotorTicks(-dY);

    if(motor1Ticks_sign > 0) printerPins->fiDirState(Pins::PinState::RESET);
    else printerPins->fiDirState(Pins::PinState::SET);

    if(motor2Ticks_sign > 0) printerPins->rDirState(Pins::PinState::SET);
    else printerPins->rDirState(Pins::PinState::RESET);

    firstMotorTicksCounter = abs(motor1Ticks_sign);
    secondMotorTicksCounter = abs(motor2Ticks_sign);

    printf("SetMove, motor1: %d, motor2: %d\r\n", firstMotorTicksCounter, secondMotorTicksCounter);

    float_t yPeriod = moveTimeInSec/secondMotorTicksCounter;
    float_t xPeriod = moveTimeInSec/firstMotorTicksCounter;

    setTIMPeriods(xPeriod, yPeriod);
}

void IRAM_ATTR DecartPrinter::firtsMotorMakeStep()
{
    if(m_state == PrinterState::PRINTING || 
    m_state == PrinterState::SEARCH_FIRST_ZERO || m_state == PrinterState::SEARCH_SECOND_ZERO || 
    m_state == PrinterState::CORRECTING_CENTER || m_state == PrinterState::ROTATE_COORD_SYS ||
    m_state == PrinterState::RESCAN_FIRST_ZERO || m_state == PrinterState::RESCAN_SECOND_ZERO)
    {
        if(firstMotorTicksCounter>0)
        {
            if(printerPins->getFiStep() == Pins::PinState::RESET)
            {
                printerPins->fiStepState(Pins::PinState::SET);

                if(printerPins->getFiDir() == Pins::PinState::RESET)
                {
                //     currentPolarPosition.fi += radOnFiTick;
                //     currentPolarPosition.r -= errRonTick;
                }
                else
                {
                //     currentPolarPosition.fi -= radOnFiTick;
                //     currentPolarPosition.r += errRonTick;
                }
            }
            else
            {
                printerPins->fiStepState(Pins::PinState::RESET);
                firstMotorTicksCounter--;
            }
        }
    }
}

void IRAM_ATTR DecartPrinter::secondMotorMakeStep()
{
    if(m_state == PrinterState::PRINTING || 
    m_state == PrinterState::SEARCH_FIRST_ZERO || m_state == PrinterState::SEARCH_SECOND_ZERO || 
    m_state == PrinterState::CORRECTING_CENTER || m_state == PrinterState::ROTATE_COORD_SYS ||
    m_state == PrinterState::RESCAN_FIRST_ZERO || m_state == PrinterState::RESCAN_SECOND_ZERO)
    {
        if(secondMotorTicksCounter>0)
        {
            if(printerPins->getRStep() == Pins::PinState::RESET)
            {

                printerPins->rStepState(Pins::PinState::SET);

                if(printerPins->getRDir() == Pins::PinState::RESET)
                {
                    // currentPolarPosition.r += mmOnRTick;
                }
                else
                {
                    // currentPolarPosition.r -= mmOnRTick;
                }
            }
            else
            {
                printerPins->rStepState(Pins::PinState::RESET);
                secondMotorTicksCounter--;
            }
        }
    }
}

int32_t DecartPrinter::lengthToMotorTicks(double_t length)
{
    return round(lengthTicksCoef * length);
}