#ifndef NETANSWER_H
#define NETANSWER_H

#include <stdint.h>

#include "netcomm/abstractcommand.hpp"
#include "netcomm/transportcommand.hpp"
#include "netcomm/playlistcommand.hpp"

void processAnswer(NetComm::AbstractCommand* recvComm, int socket);

#endif