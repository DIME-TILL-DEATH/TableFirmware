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

	do
	{
	    result = fileManager->loadNextPrint(); // Load first printRoutine
	}while(result != FM_OK);

	std::vector<GCode::GAbstractComm*> nextCommBlock;

	while(1)
    {
        if(printer->isPrinterFree())
        {
            // Thread-safe
            printer->pauseThread();
            nextCommBlock = fileManager->readNextBlock();

            if(nextCommBlock.size()>0)
            {
                for(uint16_t cnt=0; cnt<nextCommBlock.size(); cnt++)
                {
                    printer->pushPrintCommand(nextCommBlock.at(cnt));
                }
            }
            else
            {
                do
                {
                    result = fileManager->loadNextPrint();
                }
                while(result != FM_OK);
            }
            printer->resumeThread();
        }

        // handle external commands
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
