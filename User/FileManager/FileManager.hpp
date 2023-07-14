#ifndef USER_FILEMANAGER_HPP_
#define USER_FILEMANAGER_HPP_

#include "sdio.h"
#include "debug.h"
#include "FatFs/ff.h"

#include <queue>
#include <string>

typedef enum
{
    FM_OK = 0,
    FM_ERROR
}FM_RESULT;

class FileManager
{
public:
    FileManager();

    FM_RESULT connectSDCard();
private:
    FATFS fatFs;
    FIL currentPrintFile;
    std::vector<std::string> playlist;

    FM_RESULT loadPlaylist();
};

#endif /* USER_FILEMANAGER_HPP_ */
