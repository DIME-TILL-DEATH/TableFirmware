#ifndef FRAMES_H
#define FRAMES_H

#include <stdint.h>

typedef enum : uint8_t
{
    UNDEFINED = 0,
    PLAYLIST_ACTIONS,
    TRANSPORT_ACTIONS
}FrameType;

typedef struct
{
    FrameType frameType;
    uint8_t actionType;
    uint16_t frameSize;
    uint32_t frameParameters;
    uint32_t data0;
    uint32_t data1;
}FrameHeader;

union FrameHeader_uni
{
    FrameHeader structData;
    char rawData[sizeof(FrameHeader)];
};

#endif // FRAMES_H
