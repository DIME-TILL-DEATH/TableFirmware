#ifndef M51COMM_HPP
#define M51COMM_HPP

#include <string>

#include "gabstractcomm.hpp"
#include "geometry/coordinates.hpp"

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

#endif /* M51COMM_HPP */
