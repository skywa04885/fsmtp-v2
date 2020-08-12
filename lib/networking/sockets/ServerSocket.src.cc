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

ServerSocket::ServerSocket() noexcept {}

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
  int32_t flag;
  int32_t &fd = this->s_SocketFD;
  struct sockaddr_in &addr = this->s_SocketAddr;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
  }

  flag = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&flag), sizeof(flag)) == -1) {
    throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
  }

  memset(&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof (struct sockaddr)) == -1) {
    throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
  } else if (listen(fd, this->s_QueueLen) == -1) {
    throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
  }

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
      shared_ptr<ClientSocket> client = make_shared<ClientSocket>();

      try {
        client->acceptAsServer(this->s_SocketFD);

        if (this->s_SSLContext) {
          client->useSSL(this->s_SSLContext);
          client->upgradeAsServer();
        }

        thread t(this->s_Callback, client);
        t.detach();
      } catch (const runtime_error &e) {
        cerr << "Could not accept client: " << e.what() << endl;
      }
    }
  };

  if (!newThread) {
    acceptor();
  } else {
    thread acceptorThread(acceptor);
    acceptorThread.detach();
  }

  return *this;
}
