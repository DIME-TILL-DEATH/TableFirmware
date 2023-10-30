#include <memory>

#include "debug.h"

#include "FileManager.hpp"
#include "Printer.hpp"

u8 buf[512];
u8 Readbuf[512];


FileManager* fileManager;
Printer* printer;

extern "C" {
    void EXTI15_10_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

    void TIM2_IRQHandler(void) __attribute__((interrupt(/*"WCH-Interrupt-fast"*/)));
    void TIM3_IRQHandler(void) __attribute__((interrupt(/*"WCH-Interrupt-fast"*/)));
    void TIM4_IRQHandler(void) __attribute__((interrupt(/*"WCH-Interrupt-fast"*/)));
}
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SystemCoreClockUpdate();
	Delay_Init();

	USART_Printf_Init(115200);	
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID());
	printf("Paint table firmware\r\n");

	Delay_Ms(500);

	fileManager = new FileManager();
	printer = new Printer();
	FM_RESULT result;

	do
    {
        result = fileManager->connectSDCard();
        Delay_Ms(1000);
    }while(result);

	while(1)
    {
	    FM_RESULT result = fileManager->loadNextPrint();
	    if(result == FM_OK)
	    {
            std::vector<GCode::GAbstractComm*> nextCommBlock;
            do{
                while(!printer->isPrinterFree()) {}

                // Thread-safe
                printer->pauseThread();

                nextCommBlock = fileManager->readNextBlock();

                for(uint16_t cnt=0; cnt<nextCommBlock.size(); cnt++)
                {
                    GCode::GAbstractComm* aComm = nextCommBlock.at(cnt);

                    switch(aComm->commType())
                    {
                        case GCode::GCommType::M51:
                        {
                            printer->findCenter();
                            GCode::M51Comm* m51Comm = static_cast<GCode::M51Comm*>(aComm);
                            if(m51Comm)
                            {
                            }
                            break;
                        }
                        case GCode::GCommType::G1:
                        {
                            GCode::G1Comm* g1Comm = static_cast<GCode::G1Comm*>(aComm);
                            if(g1Comm)
                            {
                                printer->pushPrintPoint(g1Comm->decartCoordinates());
                            }
                            break;
                        }
                        case GCode::GCommType::G4:
                        {
                            GCode::G4Comm* g4Comm = static_cast<GCode::G4Comm*>(aComm);
                            if(g4Comm)
                            {

                            }
                            break;
                        }
                    }
                    delete aComm;
                }
                printer->resumeThread();
            }while(nextCommBlock.size()>0);
	    }
	}
}

extern "C"
{
void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_RSENS_LINE)  == SET)
    {
        printer->trigRZero();
        printf("Trigger R center\r\n");
    }

    if(EXTI_GetITStatus(EXTI_FISENS_LINE) == SET)
    {
        printer->trigFiZero();
        printf("Trigger Fi center\r\n");
    }

    EXTI_ClearITPendingBit(EXTI_FISENS_LINE | EXTI_RSENS_LINE);
}

void TIM2_IRQHandler(void)
{
    printer->makeRStep();
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

void TIM3_IRQHandler(void)
{
    printer->makeFiStep();
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}

void TIM4_IRQHandler(void)
{
    printer->printRoutine();
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}
}
