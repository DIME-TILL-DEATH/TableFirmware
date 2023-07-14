#include "FileManager.hpp"


FileManager::FileManager()
{
    SD_Init();
}

FM_RESULT FileManager::connectSDCard()
{
    SD_Error result = SD_Connect();

    if(result)
    {
        return FM_ERROR;
    }

    switch(SDCardInfo.CardType)
    {
        case SDIO_STD_CAPACITY_SD_CARD_V1_1:printf("Card Type: SDSC V1.1\r\n");break;
        case SDIO_STD_CAPACITY_SD_CARD_V2_0:printf("Card Type: SDSC V2.0\r\n");break;
        case SDIO_HIGH_CAPACITY_SD_CARD:printf("Card Type: SDHC V2.0\r\n");break;
        case SDIO_MULTIMEDIA_CARD:printf("Card Type: MMC Card\r\n");break;
    }

    printf("Card ManufacturerID: %d\r\n", SDCardInfo.SD_cid.ManufacturerID);
    printf("Card RCA: %d\r\n", SDCardInfo.RCA);
    printf("Card Capacity: %d MB\r\n",(u32)(SDCardInfo.CardCapacity>>20));
    printf("Card BlockSize: %d\r\n\r\n", SDCardInfo.CardBlockSize);


    FRESULT res;
    res = f_mount(&fatFs, "", 0);
    if(res != FR_OK)
    {
        printf("f_mount() failed, res = %d\r\n", res);
        return FM_ERROR;
    }

    DIR dir;
    res = f_opendir(&dir, "/");
    if(res != FR_OK)
    {
        printf("f_opendir() failed, res = %d\r\n", res);
        return FM_ERROR;
    }

    FILINFO fileInfo;
    uint32_t totalFiles = 0;
    uint32_t totalDirs = 0;
    printf("--------\r\nRoot directory:\r\n");
    for(;;)
    {
        res = f_readdir(&dir, &fileInfo);
        if((res != FR_OK) || (fileInfo.fname[0] == '\0'))
        {
            break;
        }

        if(fileInfo.fattrib & AM_DIR)
        {
            printf("  DIR  %s\r\n", fileInfo.fname);
            totalDirs++;
        }
        else
        {
            printf("  FILE %s\r\n", fileInfo.fname);
            totalFiles++;
        }
    }

    printf("(total: %lu dirs, %lu files)\r\n--------\r\n",
                totalDirs, totalFiles);

    res = f_closedir(&dir);
    if(res != FR_OK)
    {
        printf("f_closedir() failed, res = %d\r\n", res);
        return FM_ERROR;
    }

    loadPlaylist();

    return FM_OK;
}

FM_RESULT FileManager::loadPlaylist()
{
    FIL playlistFile;
    FRESULT res = f_open(&playlistFile, "playlist.pls", FA_READ);
    if(res != FR_OK)
    {
        printf("Open playlist failed, res = %d\r\n");
        return FM_ERROR;
    }

    TCHAR buf[256];
    while(f_gets(buf, 256, &playlistFile))
    {
        std::string currentFileName(buf);
        currentFileName.erase(currentFileName.size()-1); // remove \n
        playlist.push_back(currentFileName);
    }
    f_close(&playlistFile);

    printf("Playlist:\r\n");
    for(uint16_t i=0; i<playlist.size();i++)
    {
        printf("%s\r\n", playlist.at(i).c_str());
    }
    printf("--------\r\n");

    return FM_OK;
}
