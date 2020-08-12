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

#include "ClientSocket.src.h"

using namespace FSMTP::Sockets;

ClientSocket::ClientSocket() noexcept:
  s_SSL(nullptr), s_SSLCtx(nullptr)
{};

ClientSocket::~ClientSocket() noexcept {
  if (this->s_SSL) {
    SSL_free(this->s_SSL);
  }

  shutdown(this->s_SocketFD, SHUT_RDWR);
}

ClientSocket &ClientSocket::useSSL(SSLContext *ctx) {
  this->s_SSLCtx = ctx;
  return *this;
}

ClientSocket &ClientSocket::upgradeAsServer() {
  SSL *&ssl = this->s_SSL;
  int32_t &fd = this->s_SocketFD;

  ssl = SSL_new(this->s_SSLCtx->p_SSLCtx);
  if (!ssl) {
    throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
  }

  SSL_set_fd(ssl, fd);
  if (SSL_accept(ssl) <= 0) {
    throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
  }

  return *this;
}

ClientSocket &ClientSocket::acceptAsServer(const int32_t server) {
  int32_t &fd = this->s_SocketFD;
  struct sockaddr_in &addr = this->s_SocketAddr;
  socklen_t len = sizeof(addr);

  _accept_client:
  if ((fd = accept(server, reinterpret_cast<struct sockaddr *>(&addr), &len)) == -1) {
    this_thread::sleep_for(milliseconds(1));
    goto _accept_client;
  }

  if (!this->s_SSLCtx) return *this;
  else return this->upgradeAsServer();
}

void ClientSocket::write(const char *msg, const size_t len) {
  if (this->s_SSLCtx) {
    if (SSL_write(this->s_SSL, msg, len) <= 0) {
      throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
    }
  } else {
    if (send(this->s_SocketFD, msg, len, 0) == -1) {
      throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
    }
  }
}

#define _CLIENT_SOCKET_RBUF_LEN 255
string read(const char *delim) {
  char buffer[_CLIENT_SOCKET_RBUF_LEN];
}
