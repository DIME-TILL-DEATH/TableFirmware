#include "g4Comm.hpp"

using namespace GCode;

G4Comm::G4Comm(uint32_t pause)
{
    m_commType = GCommType::G4;

    m_pause = pause;
}

uint32_t G4Comm::pause()
{
    return m_pause;
}