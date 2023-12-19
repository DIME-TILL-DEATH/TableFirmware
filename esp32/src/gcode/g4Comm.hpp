#ifndef G4COMM_HPP
#define G4COMM_HPP

#include <stdint.h>

#include "gabstractcomm.hpp"
#include "geometry/coordinates.hpp"

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




#endif /* G4COMM_HPP */
