#ifndef USER_FILEMANAGER_HPP_
#define USER_FILEMANAGER_HPP_

#include <queue>
#include <string>
#include <memory>
#include <stdexcept>

#include "debug.h"
#include "sdio.h"

#include "FatFs/ff.h"

#include "Coordinates.hpp"
#include "GCode/GAbstractComm.hpp"
#include "GCode/M51Comm.hpp"
#include "GCode/G1Comm.hpp"
#include "GCode/G4Comm.hpp"

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
    FM_RESULT loadNextPrint();

    static constexpr uint8_t blockSize = 32;

    std::vector<GCode::GAbstractComm*> readNextBlock();
private:
    FATFS fatFs;
    FIL currentPrintFile;
    int16_t curPlsPos{-1};
    std::vector<std::string> playlist;

    FM_RESULT loadPlaylist();
};

#endif /* USER_FILEMANAGER_HPP_ */
