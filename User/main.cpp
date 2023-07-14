#include "debug.h"

#include "FileManager.hpp"


u8 buf[512];
u8 Readbuf[512];


FileManager* fileManager;
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

	fileManager = new FileManager();
	FM_RESULT result;

	do
    {
        result = fileManager->connectSDCard();
        delay_ms(1000);
    }while(result);

	while(1)
    {

	}
}

