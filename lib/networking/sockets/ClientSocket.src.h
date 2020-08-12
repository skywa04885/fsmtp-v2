/*
	Copyright [2020] [Luke A.C.A. Rieff]

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#ifndef _LIB_NETWORKING_SOCKETS_SOCKET_H
#define _LIB_NETWORKING_SOCKETS_SOCKET_H

#include "../../default.h"
#include "./SSLContext.src.h"
#include "../../general/macros.src.h"

namespace FSMTP::Sockets {
  class ClientSocket {
  public:
    ClientSocket() noexcept;
    ~ClientSocket() noexcept;

    ClientSocket &upgradeAsServer();
    ClientSocket &useSSL(SSLContext *ctx);
    ClientSocket &acceptAsServer(const int32_t server);
    void write(const char *msg, const size_t len);

  private:
    struct sockaddr_in s_SocketAddr;
    SSLContext *s_SSLCtx;
    SSL *s_SSL;
    int32_t s_SocketFD;
  };
}

#endif