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

	// TODO: SD-card disconnect and reconnect interrupt!
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
                NVIC_DisableIRQ(TIM2_IRQn);
                NVIC_DisableIRQ(TIM3_IRQn);
                NVIC_DisableIRQ(TIM4_IRQn);

                nextCommBlock = fileManager->readNextBlock();

                printf("\r\nNext comm block size: %d\r\n\r\n", nextCommBlock.size());

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
//                                delete m51Comm;
                            }
                            break;
                        }
                        case GCode::GCommType::G1:
                        {
                            GCode::G1Comm* g1Comm = static_cast<GCode::G1Comm*>(aComm);
                            if(g1Comm)
                            {
                                printer->pushPrintPoint(g1Comm->decartCoordinates());
//                                delete g1Comm;
                            }
                            break;
                        }
                        case GCode::GCommType::G4:
                        {
                            GCode::G4Comm* g4Comm = static_cast<GCode::G4Comm*>(aComm);
                            if(g4Comm)
                            {
                               // printf("File printed.\r\n");
//                                Delay_Ms(1000);
//                                delete g4Comm;
                            }
                            break;
                        }
                    }
                    delete aComm;
                }

                NVIC_EnableIRQ(TIM2_IRQn);
                NVIC_EnableIRQ(TIM3_IRQn);
                NVIC_EnableIRQ(TIM4_IRQn);

            }while(nextCommBlock.size()>0);
//            printf("Cycle out\r\n");
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
