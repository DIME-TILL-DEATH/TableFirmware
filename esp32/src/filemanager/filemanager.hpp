#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <queue>
#include <string>
#include <memory>
#include <stdexcept>

#include "ff.h"

#include "geometry/coordinates.hpp"
#include "gcode/gabstractcomm.hpp"
#include "gcode/m51comm.hpp"
#include "gcode/g1comm.hpp"
#include "gcode/g4comm.hpp"

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
    FM_RESULT loadPlaylist();
    FM_RESULT loadNextPrint();

    static constexpr uint8_t blockSize = 32;

    std::vector<GCode::GAbstractComm*> readNextBlock();
private:
    
    FILE* currentPrintFile;
    int16_t curPlsPos{-1};
    std::vector<std::string> playlist;
};

#endif /* USER_FILEMANAGER_HPP */
