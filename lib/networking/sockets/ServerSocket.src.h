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

#ifndef _LIB_NETWORKING_SOCKETS_SERVER_SOCKET_H
#define _LIB_NETWORKING_SOCKETS_SERVER_SOCKET_H

#include "../../default.h"
#include "../../general/Logger.src.h"
#include "SSLContext.src.h"
#include "ClientSocket.src.h"

namespace FSMTP::Sockets {
  class ServerSocket {
  public:
    ServerSocket() noexcept;
    ~ServerSocket() noexcept;

    ServerSocket &useSSL(SSLContext *sslCtx);
    ServerSocket &queue(const int32_t queueLen);
    ServerSocket &listenServer(const int32_t port);
    ServerSocket &handler(function<void(shared_ptr<ClientSocket>)> callback);
    ServerSocket &startAcceptor(const bool newThread);
  private:
    SSLContext *s_SSLContext;
    struct sockaddr_in s_SocketAddr;
    int32_t s_SocketFD, s_QueueLen;
    function<void(shared_ptr<ClientSocket>)> s_Callback;
  };
}

#endif