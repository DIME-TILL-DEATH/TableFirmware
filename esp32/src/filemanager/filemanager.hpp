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
    FM_RESULT loadPrintFromPlaylist(uint16_t num);

    static constexpr uint8_t blockSize = 32;
    uint16_t pointsNum() {return m_pointsNum;}

    GCode::GAbstractComm* readNextComm();

    void changePlaylist(const std::vector<std::string>* newPlaylist);
    void changePlaylistPos(int16_t newPos);

    std::vector<std::string>* getPlaylist_ptr() {return &playlist;};
    int16_t getCurrentPosition() {return curPlsPos;};

    constexpr static std::string mountPoint = "/sdcard/";
private:
    
    FILE* currentPrintFile;
    int16_t curPlsPos{-1};
    std::vector<std::string> playlist;
    uint16_t m_pointsNum{0};
};

#endif /* USER_FILEMANAGER_HPP */
