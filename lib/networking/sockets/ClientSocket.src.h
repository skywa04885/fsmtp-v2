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
#include "../../dns/Resolver.src.h"

namespace FSMTP::Sockets {
  union __SockAddrReturnP {
    struct sockaddr_in *ipv4;
    struct sockaddr_in6 *ipv6;
  };

  enum SocketAddrType {
    SockAddrType_IPv6,
    SockAddrType_IPv4
  };

  class ClientSocket {
  public:
    ClientSocket() noexcept;
    ClientSocket(SocketAddrType type) noexcept;
    ~ClientSocket() noexcept;

    ClientSocket &upgradeAsServer();
    ClientSocket &upgradeAsClient();
    ClientSocket &useSSL(SSLContext *ctx);
    ClientSocket &acceptAsServer(const int32_t server);
    ClientSocket &connectAsClient(const char *host, const int32_t port);
    ClientSocket &timeout(const int32_t s);
    string getPrefix();
    string getReverseLookup();
    string getString();
    __SockAddrReturnP getAddress();
    int32_t write(const char *msg, const size_t len);
    int32_t write(const std::string &msg);
    string readToDelim(const char *delim);
    int32_t read(char *buffer, const size_t bufferSize);
    int32_t peek(char *buffer, const size_t bufferSize);
    int32_t getPort();
    bool usingSSL();
  private:
    union {
      struct sockaddr_in6 m_IPv6Addr;
      struct sockaddr_in m_IPv4Addr;
    };

    SocketAddrType m_Type;

    SSLContext *s_SSLCtx;
    SSL *s_SSL;
    int32_t s_SocketFD;
  };
}

#endif