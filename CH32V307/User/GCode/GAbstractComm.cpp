#include "GAbstractComm.hpp"

using namespace GCode;


GAbstractComm::GAbstractComm()
{
    m_commType = GCommType::ABSTRACT;
}

GCommType GAbstractComm::commType()
{
    return m_commType;
}



