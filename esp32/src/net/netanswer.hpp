#ifndef NETANSWER_H
#define NETANSWER_H

#include <stdint.h>

#include "netcomm/abstractcommand.hpp"
#include "netcomm/hardwarecommand.hpp"
#include "netcomm/playlistcommand.hpp"
#include "netcomm/filecommand.hpp"
#include "netcomm/firmwarecommand.hpp"

void processAnswer(NetComm::AbstractCommand* recvComm, int socket);

#endif