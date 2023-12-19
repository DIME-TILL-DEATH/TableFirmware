#ifndef __SDIO_H
#define __SDIO_H	

#ifdef __cplusplus
 extern "C" {
#endif

#include "debug.h"

/* SDIO clock */
#define SDIO_INIT_CLK_DIV        238
#define SDIO_TRANSFER_CLK_DIV    0x00

/* SDIO work mode */
#define SD_POLLING_MODE     0
#define SD_DMA_MODE         1

/*SDIO Err define */
typedef enum
{
    /* special err */
    SD_CMD_CRC_FAIL                    = (1),
    SD_DATA_CRC_FAIL                   = (2),
    SD_CMD_RSP_TIMEOUT                 = (3),
    SD_DATA_TIMEOUT                    = (4),
    SD_TX_UNDERRUN                     = (5),
    SD_RX_OVERRUN                      = (6),
    SD_START_BIT_ERR                   = (7),
    SD_CMD_OUT_OF_RANGE                = (8),
    SD_ADDR_MISALIGNED                 = (9),
    SD_BLOCK_LEN_ERR                   = (10),
    SD_ERASE_SEQ_ERR                   = (11),
    SD_BAD_ERASE_PARAM                 = (12),
    SD_WRITE_PROT_VIOLATION            = (13),
    SD_LOCK_UNLOCK_FAILED              = (14),
    SD_COM_CRC_FAILED                  = (15),
    SD_ILLEGAL_CMD                     = (16),
    SD_CARD_ECC_FAILED                 = (17),
    SD_CC_ERROR                        = (18),
    SD_GENERAL_UNKNOWN_ERROR           = (19),
    SD_STREAM_READ_UNDERRUN            = (20),
    SD_STREAM_WRITE_OVERRUN            = (21),
    SD_CID_CSD_OVERWRITE               = (22),
    SD_WP_ERASE_SKIP                   = (23),
    SD_CARD_ECC_DISABLED               = (24),
    SD_ERASE_RESET                     = (25),
    SD_AKE_SEQ_ERROR                   = (26),
    SD_INVALID_VOLTRANGE               = (27),
    SD_ADDR_OUT_OF_RANGE               = (28),
    SD_SWITCH_ERROR                    = (29),
    SD_SDIO_DISABLED                   = (30),
    SD_SDIO_FUNCTION_BUSY              = (31),
    SD_SDIO_FUNCTION_FAILED            = (32),
    SD_SDIO_UNKNOWN_FUNCTION           = (33),
    /* Standard err */
    SD_INTERNAL_ERROR,
    SD_NOT_CONFIGURED,
    SD_REQUEST_PENDING,
    SD_REQUEST_NOT_APPLICABLE,
    SD_INVALID_PARAMETER,
    SD_UNSUPPORTED_FEATURE,
    SD_UNSUPPORTED_HW,
    SD_ERROR,
    SD_OK = 0
} SD_Error;

/*SD——CSD*/
typedef struct
{
    uint8_t  CSDStruct;
    uint8_t  SysSpecVersion;
    uint8_t  Reserved1;
    uint8_t  TAAC;
    uint8_t  NSAC;
    uint8_t  MaxBusClkFrec;
    uint16_t CardComdClasses;
    uint8_t  RdBlockLen;
    uint8_t  PartBlockRead;
    uint8_t  WrBlockMisalign;
    uint8_t  RdBlockMisalign;
    uint8_t  DSRImpl;
    uint8_t  Reserved2;
    uint32_t DeviceSize;
    uint8_t  MaxRdCurrentVDDMin;
    uint8_t  MaxRdCurrentVDDMax;
    uint8_t  MaxWrCurrentVDDMin;
    uint8_t  MaxWrCurrentVDDMax;
    uint8_t  DeviceSizeMul;
    uint8_t  EraseGrSize;
    uint8_t  EraseGrMul;
    uint8_t  WrProtectGrSize;
    uint8_t  WrProtectGrEnable;
    uint8_t  ManDeflECC;
    uint8_t  WrSpeedFact;
    uint8_t  MaxWrBlockLen;
    uint8_t  WriteBlockPaPartial;
    uint8_t  Reserved3;
    uint8_t  ContentProtectAppli;
    uint8_t  FileFormatGrouop;
    uint8_t  CopyFlag;
    uint8_t  PermWrProtect;
    uint8_t  TempWrProtect;
    uint8_t  FileFormat;
    uint8_t  ECC;
    uint8_t  CSD_CRC;
    uint8_t  Reserved4;
} SD_CSD;   

/*SD——CID*/
typedef struct
{
    uint8_t  ManufacturerID;
    uint16_t OEM_AppliID;
    uint32_t ProdName1;
    uint8_t  ProdName2;
    uint8_t  ProdRev;
    uint32_t ProdSN;
    uint8_t  Reserved1;
    uint16_t ManufactDate;
    uint8_t  CID_CRC;
    uint8_t  Reserved2;
} SD_CID;

/* SD statue */
typedef enum
{
    SD_CARD_READY                  = ((uint32_t)0x00000001),
    SD_CARD_IDENTIFICATION         = ((uint32_t)0x00000002),
    SD_CARD_STANDBY                = ((uint32_t)0x00000003),
    SD_CARD_TRANSFER               = ((uint32_t)0x00000004),
    SD_CARD_SENDING                = ((uint32_t)0x00000005),
    SD_CARD_RECEIVING              = ((uint32_t)0x00000006),
    SD_CARD_PROGRAMMING            = ((uint32_t)0x00000007),
    SD_CARD_DISCONNECTED           = ((uint32_t)0x00000008),
    SD_CARD_ERROR                  = ((uint32_t)0x000000FF)
}SDCardState;

/* SD informatiion */
typedef struct
{
  SD_CSD SD_csd;
  SD_CID SD_cid;
  long long CardCapacity;
  uint32_t CardBlockSize;
  uint16_t RCA;
  uint8_t CardType;
} SD_CardInfo;
extern SD_CardInfo SDCardInfo;

/* SDIO instruction*/
#define SD_CMD_GO_IDLE_STATE                       ((uint8_t)0)
#define SD_CMD_SEND_OP_COND                        ((uint8_t)1)
#define SD_CMD_ALL_SEND_CID                        ((uint8_t)2)
#define SD_CMD_SET_REL_ADDR                        ((uint8_t)3)
#define SD_CMD_SET_DSR                             ((uint8_t)4)
#define SD_CMD_SDIO_SEN_OP_COND                    ((uint8_t)5)
#define SD_CMD_HS_SWITCH                           ((uint8_t)6)
#define SD_CMD_SEL_DESEL_CARD                      ((uint8_t)7)
#define SD_CMD_HS_SEND_EXT_CSD                     ((uint8_t)8)
#define SD_CMD_SEND_CSD                            ((uint8_t)9)
#define SD_CMD_SEND_CID                            ((uint8_t)10)
#define SD_CMD_READ_DAT_UNTIL_STOP                 ((uint8_t)11)
#define SD_CMD_STOP_TRANSMISSION                   ((uint8_t)12)
#define SD_CMD_SEND_STATUS                         ((uint8_t)13)
#define SD_CMD_HS_BUSTEST_READ                     ((uint8_t)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((uint8_t)15)
#define SD_CMD_SET_BLOCKLEN                        ((uint8_t)16)
#define SD_CMD_READ_SINGLE_BLOCK                   ((uint8_t)17)
#define SD_CMD_READ_MULT_BLOCK                     ((uint8_t)18)
#define SD_CMD_HS_BUSTEST_WRITE                    ((uint8_t)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP                ((uint8_t)20)
#define SD_CMD_SET_BLOCK_COUNT                     ((uint8_t)23)
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((uint8_t)24)
#define SD_CMD_WRITE_MULT_BLOCK                    ((uint8_t)25)
#define SD_CMD_PROG_CID                            ((uint8_t)26)
#define SD_CMD_PROG_CSD                            ((uint8_t)27)
#define SD_CMD_SET_WRITE_PROT                      ((uint8_t)28)
#define SD_CMD_CLR_WRITE_PROT                      ((uint8_t)29)
#define SD_CMD_SEND_WRITE_PROT                     ((uint8_t)30)
#define SD_CMD_SD_ERASE_GRP_START                  ((uint8_t)32)
#define SD_CMD_SD_ERASE_GRP_END                    ((uint8_t)33)
#define SD_CMD_ERASE_GRP_START                     ((uint8_t)35)
#define SD_CMD_ERASE_GRP_END                       ((uint8_t)36)
#define SD_CMD_ERASE                               ((uint8_t)38)
#define SD_CMD_FAST_IO                             ((uint8_t)39)
#define SD_CMD_GO_IRQ_STATE                        ((uint8_t)40)
#define SD_CMD_LOCK_UNLOCK                         ((uint8_t)42)
#define SD_CMD_APP_CMD                             ((uint8_t)55)
#define SD_CMD_GEN_CMD                             ((uint8_t)56)
#define SD_CMD_NO_CMD                              ((uint8_t)64)

/*Following commands are SD Card Specific commands.
  SDIO_APP_CMD ：CMD55 should be sent before sending these commands.
*/
#define SD_CMD_APP_SD_SET_BUSWIDTH                 ((uint8_t)6)
#define SD_CMD_SD_APP_STAUS                        ((uint8_t)13)
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS        ((uint8_t)22)
#define SD_CMD_SD_APP_OP_COND                      ((uint8_t)41)
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT          ((uint8_t)42)
#define SD_CMD_SD_APP_SEND_SCR                     ((uint8_t)51)
#define SD_CMD_SDIO_RW_DIRECT                      ((uint8_t)52)
#define SD_CMD_SDIO_RW_EXTENDED                    ((uint8_t)53)

/*Following commands are SD Card Specific security commands.
  SDIO_APP_CMD should be sent before sending these commands.
*/
#define SD_CMD_SD_APP_GET_MKB                      ((uint8_t)43)
#define SD_CMD_SD_APP_GET_MID                      ((uint8_t)44)
#define SD_CMD_SD_APP_SET_CER_RN1                  ((uint8_t)45)
#define SD_CMD_SD_APP_GET_CER_RN2                  ((uint8_t)46)
#define SD_CMD_SD_APP_SET_CER_RES2                 ((uint8_t)47)
#define SD_CMD_SD_APP_GET_CER_RES1                 ((uint8_t)48)
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((uint8_t)18)
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((uint8_t)25)
#define SD_CMD_SD_APP_SECURE_ERASE                 ((uint8_t)38)
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA           ((uint8_t)49)
#define SD_CMD_SD_APP_SECURE_WRITE_MKB             ((uint8_t)48)

/* SD definition */
#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)

/* SDIO parameter definition */
#define SDIO_STATIC_FLAGS               ((uint32_t)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((uint32_t)0x00010000)
#define SDIO_DATATIMEOUT                ((uint32_t)0xFFFFFFFF)
#define SDIO_FIFO_Address               ((uint32_t)0x40018080)

/* Mask for errors Card Status R1 (OCR Register) */
#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

/* Masks for R6 Response */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define SD_STD_CAPACITY                 ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)
#define SD_VOLTAGE_WINDOW_MMC           ((uint32_t)0x80FF8000)

#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)
#define SD_CARD_PROGRAMMING             ((uint32_t)0x00000007)
#define SD_CARD_RECEIVING               ((uint32_t)0x00000006)
#define SD_DATATIMEOUT                  ((uint32_t)0xFFFFFFFF)
#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)

/*Command Class Supported */
#define SD_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)

/*CMD8*/
#define SDIO_SEND_IF_COND               ((uint32_t)0x00000008)

void SD_Init();
SD_Error SD_Connect();
void SDIO_Clock_Set(uint8_t clkdiv);
void SDIO_Send_Cmd(uint8_t cmdindex, uint8_t waitrsp, uint32_t arg);
void SDIO_Send_Data_Cfg(uint32_t datatimeout, uint32_t datalen, uint8_t blksize, uint8_t dir);
SD_Error SD_PowerON(void);    
SD_Error SD_PowerOFF(void);
SD_Error SD_InitializeCards(void);
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo);
SD_Error SD_EnableWideBusOperation(uint32_t wmode);
SD_Error SD_SetDeviceMode(uint32_t mode);
SD_Error SD_SelectDeselect(uint32_t addr);
SD_Error SD_SendStatus(uint32_t *pcardstatus);
SDCardState SD_GetState(void);
SD_Error SD_ReadBlock(uint8_t *buf, long long addr, uint16_t blksize);
SD_Error SD_ReadMultiBlocks(uint8_t *buf, long long  addr, uint16_t blksize, uint32_t nblks);
SD_Error SD_WriteBlock(uint8_t *buf, long long addr, uint16_t blksize);
SD_Error SD_WriteMultiBlocks(uint8_t *buf, long long addr, uint16_t blksize, uint32_t nblks);
SD_Error SD_ProcessIRQSrc(void);
void SD_DMA_Config(uint32_t*mbuf, uint32_t bufsize, uint32_t dir);
uint8_t SD_ReadDisk(uint8_t*buf, uint32_t sector, uint8_t cnt);
uint8_t SD_WriteDisk(uint8_t*buf, uint32_t sector, uint8_t cnt);

#ifdef __cplusplus
}
#endif

#endif









