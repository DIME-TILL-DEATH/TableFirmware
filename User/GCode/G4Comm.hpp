#ifndef USER_GCODE_G4COMM_HPP_
#define USER_GCODE_G4COMM_HPP_

#include "debug.h"

#include "GAbstractComm.hpp"
#include "Coordinates.hpp"

namespace GCode
{

class G4Comm : public GAbstractComm
{
public:
    G4Comm(uint32_t pause);

    uint32_t pause();
private:
    uint32_t m_pause;
};


};




#endif /* USER_GCODE_G4COMM_HPP_ */
