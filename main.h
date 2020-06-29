#include <iostream>
#include <memory>
#include <cstdint>
#include <bitset>

#include "lib/networking/SMTPSocket.src.h"
#include "lib/server/SMTPServer.src.h"
#include "lib/smtp/Response.src.h"

#pragma once

using namespace FSMTP;
using Networking::SMTPSocketType;
using Networking::SMTPSocket;
using Server::SMTPServer;

int main(const int argc, const char **argv);