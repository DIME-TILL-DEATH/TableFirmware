#ifndef USER_GCODE_M51COMM_HPP_
#define USER_GCODE_M51COMM_HPP_

#include <string>

#include "GAbstractComm.hpp"
#include "Coordinates.hpp"

namespace GCode
{

class M51Comm : public GAbstractComm
{
public:
    M51Comm(std::string param);

private:
    std::string m_parameter;
};

};

#endif /* USER_GCODE_M51COMM_HPP_ */
