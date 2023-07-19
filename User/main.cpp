#include <memory>

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
	        std::vector<GCode::GAbstractComm*> answer;

	        do{
                answer = fileManager->readNextBlock();

                for(uint16_t cnt=0; cnt<answer.size(); cnt++)
                {
                    GCode::GAbstractComm* aComm = answer.at(cnt);

                    switch(aComm->commType())
                    {
                        case GCode::GCommType::M51:
                        {

                            break;
                        }
                        case GCode::GCommType::G1:
                        {
                            GCode::G1Comm* g1Comm = static_cast<GCode::G1Comm*>(aComm);

                            if(g1Comm)
                            {


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
                    Delay_Ms(500);
                }
	        }while(answer.size()>0);
	    }
	}
}

