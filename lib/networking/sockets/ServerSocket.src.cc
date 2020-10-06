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

#include "ServerSocket.src.h"

using namespace FSMTP::Sockets;

ServerSocket::ServerSocket() noexcept:
  s_SSLContext(nullptr), m_Type(ServerSocketAddrType::ServerSocketAddr_IPv4)
{}

ServerSocket::ServerSocket(ServerSocketAddrType type):
  s_SSLContext(nullptr), m_Type(type)
{}

ServerSocket::~ServerSocket() noexcept {
  shutdown(this->s_SocketFD, SHUT_RDWR);
}

ServerSocket &ServerSocket::useSSL(SSLContext *sslCtx) {
  this->s_SSLContext = sslCtx;
  return *this;
}

ServerSocket &ServerSocket::queue(const int32_t queueLen) {
  this->s_QueueLen = queueLen;
  return *this;
}

ServerSocket &ServerSocket::listenServer(const int32_t port) {
  int32_t &fd = this->s_SocketFD;
  int32_t flag;

  // Creates the socket FD with either AF_INET or AF_INET6
  int32_t type = 0;
  switch (this->m_Type) {
    case ServerSocketAddrType::ServerSocketAddr_IPv4: type = AF_INET; break;
    case ServerSocketAddrType::ServerSocketAddr_IPv6: type = AF_INET6; break;
    default: throw runtime_error(EXCEPT_DEBUG("Invalid address enum"));
  }
  fd = socket(type, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    throw runtime_error(EXCEPT_DEBUG(
      string("Could not creat esocket: ") + strerror(errno)));

  // Prepares the server socket address for listening
  //  we will set stuff like the port here
  switch (this->m_Type) {
    case ServerSocketAddrType::ServerSocketAddr_IPv4:
      memset(&this->m_IPv4Addr, 0, sizeof (struct sockaddr_in));
      this->m_IPv4Addr.sin_family = AF_INET;
      this->m_IPv4Addr.sin_port = htons(port);
      this->m_IPv4Addr.sin_addr.s_addr = INADDR_ANY;
      break;
    case ServerSocketAddrType::ServerSocketAddr_IPv6:
      memset(&this->m_IPv6Addr, 0, sizeof (struct sockaddr_in6));
      this->m_IPv6Addr.sin6_family = AF_INET6;
      this->m_IPv6Addr.sin6_port = htons(port);
      this->m_IPv6Addr.sin6_addr = in6addr_any;
      break;
    default: throw runtime_error(EXCEPT_DEBUG("Invalid address enum"));
  }

  // Sets the reuse address flag, so we may re-use existing addresses
  flag = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&flag), sizeof(flag)) == -1)
    throw runtime_error(EXCEPT_DEBUG(
      string("setsockopt(SO_REUSEADDR) failed: ") + strerror(errno)));

  // Gets the struct sockaddr * variable, we also get the length
  //  of the structure, required for binding
  struct sockaddr *addr = nullptr;
  socklen_t len = 0;
  switch (this->m_Type) {
    case ServerSocketAddrType::ServerSocketAddr_IPv4:
      addr = reinterpret_cast<struct sockaddr *>(&this->m_IPv4Addr);
      len = sizeof(struct sockaddr_in);
      break;
    case ServerSocketAddrType::ServerSocketAddr_IPv6:
      addr = reinterpret_cast<struct sockaddr *>(&this->m_IPv6Addr);
      len = sizeof(struct sockaddr_in6);
      break;
    default: throw runtime_error(EXCEPT_DEBUG("Invalid address enum"));
  }

  // Binds the socket, after which we check if any error occured
  //  if so we will throw the error
  if (bind(fd, addr, len) == -1)
    throw runtime_error(EXCEPT_DEBUG(
      string("bind() failed: ") + strerror(errno)));

  // Listens the socket, and throws an error if this fails
  if (listen(fd, this->s_QueueLen) == -1)
    throw runtime_error(EXCEPT_DEBUG(
      string("listen() failed: ") + strerror(errno)));
  
  return *this;
}

ServerSocket &ServerSocket::handler(function<void(shared_ptr<ClientSocket>)> callback) {
  this->s_Callback = callback;
  return *this;
}

ServerSocket &ServerSocket::startAcceptor(const bool newThread)
{
  auto acceptor = [&]() {
    for (;;) {
      shared_ptr<ClientSocket> client;

      // Creates the client based uppon the ip address type we're using
      //  currently only IPv4 and IPv6
      switch (this->m_Type) {
        case ServerSocketAddrType::ServerSocketAddr_IPv4:
          client = make_shared<ClientSocket>(Networking::IP::Protocol::Protocol_IPv4); break;
        case ServerSocketAddrType::ServerSocketAddr_IPv6:
          client = make_shared<ClientSocket>(Networking::IP::Protocol::Protocol_IPv6); break;
        default: throw runtime_error("Invalid address enum");
      }
      
      // Starts accepting an socket to the just created
      //  client socket
      try {
        client->acceptAsServer(this->s_SocketFD);
        if (this->s_SSLContext) {
          client->useSSL(this->s_SSLContext).upgradeAsServer();
        }

        client->timeout(7);

        thread t(this->s_Callback, client);
        t.detach();
      } catch (...) {}
    }
  };

  // If the newThread boolean is set, we want to create an
  //  separate thread with the acceptor, if this is not the
  //  case we just call the lambda, and run the code in the
  //  current thread

  if (!newThread) {
    acceptor();
  } else {
    thread acceptorThread(acceptor);
    acceptorThread.detach();
  }

  return *this;
}
