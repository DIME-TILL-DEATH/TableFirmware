#ifndef NETANSWER_H
#define NETANSWER_H

#include <stdint.h>

#include "netcomm/abstractcommand.hpp"
#include "netcomm/transportcommand.hpp"
#include "netcomm/playlistcommand.hpp"

void formAnswer(NetComm::AbstractCommand* recvComm, uint8_t* txBuffer, size_t* answerLen);

#endif