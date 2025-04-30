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
    firstCoordEndstopTrigger = true;
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
    secondCoordEndstopTrigger = true;
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
                setState(PrinterState::IDLE);
                return; //skip point
            }

            float_t deltaX = targetPosition.x - currentPosition.x;
            float_t deltaY = targetPosition.y - currentPosition.y;

            stepX = deltaX;// / steps;
            stepY = deltaY;// / steps;

            stepTime =  (sqrt(pow((stepX), 2) + pow((stepY), 2)));// / speed;

            // INFO block==========
            //    printf("Point num: %d\r\n", pointNum);

            //    printf("DECART: current(%lf, %lf), target(%lf, %lf)\r\n", currentPosition.x, currentPosition.y, targetPosition.x, targetPosition.y);
            //    printf("stepX: %lf, stepY: %lf\r\n", stepX, stepY);
            //    printf("\r\n");
            //=================================

            pointNum++;

            setMove(stepX, stepY, stepTime/speed);
            setState(PrinterState::PRINTING);
            // printRoutine();

            gpio_set_level(GPIO_NUM_23, 0);
            break;
        }

        case PrinterState::PRINTING:
        {
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
                setMove(0, yMoveDiapason * 1.1 / printScaleCoef, 10); // 10% margin
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

                setMove(xMoveDiapason/2/printScaleCoef, -yMoveDiapason/2/printScaleCoef + 5, 5);

                setState(PrinterState::CORRECTING_CENTER);
                break;
            }
            else if(secondMotorTicksCounter == 0)
            {
                ESP_LOGE(TAG, "Y edge not found! Pad is out of bounds.\r\n");
                setMove(0, -yMoveDiapason/10, 10);
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
    int32_t motor1Ticks_sign = lengthToMotorTicks(dX) - lengthToMotorTicks(-dY);
    int32_t motor2Ticks_sign = lengthToMotorTicks(dX) + lengthToMotorTicks(-dY);

    if(motor1Ticks_sign > 0) printerPins->setFirstMotorDirState(Pins::PinState::RESET);
    else printerPins->setFirstMotorDirState(Pins::PinState::SET);

    if(motor2Ticks_sign > 0) printerPins->setSecondMotorDirState(Pins::PinState::SET);
    else printerPins->setSecondMotorDirState(Pins::PinState::RESET);

    firstMotorTicksCounter = abs(motor1Ticks_sign);
    secondMotorTicksCounter = abs(motor2Ticks_sign);

    // printf("SetMove, motor1: %d, motor2: %d\r\n", firstMotorTicksCounter, secondMotorTicksCounter);

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
            if(printerPins->getFirstMotorStep() == Pins::PinState::RESET)
            {
                printerPins->setFirstMotorStepState(Pins::PinState::SET);
            }
            else
            {
                printerPins->setFirstMotorStepState(Pins::PinState::RESET);
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
            if(printerPins->getSecondMotorStep() == Pins::PinState::RESET)
            {
                printerPins->setSecondMotorStepState(Pins::PinState::SET);
            }
            else
            {
                printerPins->setSecondMotorStepState(Pins::PinState::RESET);
                secondMotorTicksCounter--;
            }
        }
    }
}

int32_t DecartPrinter::lengthToMotorTicks(double_t length)
{
    return round(lengthTicksCoef * length);
}