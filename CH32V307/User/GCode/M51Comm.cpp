#include "M51Comm.hpp"

using namespace GCode;


M51Comm::M51Comm(std::string param)
{
    m_commType = GCommType::M51;

    m_parameter = param;
}
