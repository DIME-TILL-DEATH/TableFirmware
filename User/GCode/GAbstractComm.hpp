#ifndef GABSTRACTCOMM_HPP_
#define GABSTRACTCOMM_HPP_

#include "debug.h"

namespace GCode
{

typedef enum
{
    ABSTRACT = 0,
    G1 = 1,
    G4 = 4,
    M51 = 51
}GCommType;

class GAbstractComm
{
public:
    GAbstractComm();
    GCommType commType();
protected:
    GCommType m_commType;
};


};

#endif /* GABSTRACTCOMM_HPP_ */
