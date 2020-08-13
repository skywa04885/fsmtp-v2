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
  auto *&ctx = this->s_SSLCtx;

  assert(("SSLContext does not exist", ctx != nullptr));

  ssl = SSL_new(ctx->p_SSLCtx);
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

  // Attempts to accept an client, if this fails we wait for some
  //  time and then jump back to the accept phase. When this is
  //  done we check if the SSLCtx is not null, if this is true
  //  we know that we use ssl, and want to upgrade first

  _accept_client:
  if ((fd = accept(server, reinterpret_cast<struct sockaddr *>(&addr), &len)) == -1) {
    this_thread::sleep_for(milliseconds(1));
    goto _accept_client;
  }

  if (!this->s_SSLCtx) return *this;
  else return this->upgradeAsServer();
}

int32_t ClientSocket::write(const char *msg, const size_t len) {
  int32_t rc;

  if (this->s_SSLCtx) {
    if ((rc = SSL_write(this->s_SSL, msg, len)) < 0) {
      throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
    }
  } else {
    if ((rc = send(this->s_SocketFD, msg, len, 0)) < 0) {
      throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
    }
  }

  return rc;
}

int32_t ClientSocket::write(const std::string &msg) {
  return this->write(msg.c_str(), msg.length());
}

int32_t ClientSocket::read(char *buffer, const size_t bufferSize) {
  int32_t rc;

  if (this->s_SSLCtx) {
    if ((rc = SSL_read(this->s_SSL, buffer, bufferSize)) <= 0) {
      throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
    }
  } else {
    if ((rc = recv(this->s_SocketFD, buffer, bufferSize, 0)) == -1) {
      throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
    }
  }

  return rc;
}

int32_t ClientSocket::peek(char *buffer, const size_t bufferSize) {
  int32_t rc;

  if (this->s_SSLCtx) {
    if ((rc = SSL_peek(this->s_SSL, buffer, bufferSize)) <= 0) {
      throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
    }
  } else {
    if ((rc = recv(this->s_SocketFD, buffer, bufferSize, MSG_PEEK)) <= 0) {
      throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
    }
  }

  return rc;
}

string ClientSocket::readToDelim(const char *delim) {
  size_t delimSize = strlen(delim), searchIndex;
  size_t bufferSize = delimSize + 255;
  char *buffer = new char[bufferSize];
  bool endFound = false;
  int32_t readLen;
  string result;

  // Keeps reading from the client untill delimiter is reached
  //  then we return from the method. We use peek so we can
  //  later use pipelining. When finally adding the buffer to the
  //  string, we subtract the delimiter sice ( ony when end reached )

  while (endFound == false) {
    readLen = this->peek(buffer, bufferSize);

    string searchString;
    if (result.length() > delimSize) {
      searchString += result.substr(result.length() - delimSize);
    }

    searchString.append(buffer, readLen);
    if ((searchIndex = searchString.find(delim)) != string::npos) {
      endFound = true;
    }

    readLen = this->read(buffer, searchIndex + delimSize);
    result.append(buffer, readLen);
  }

  delete[] buffer;
  return result.substr(0, result.length() - delimSize);
}

bool ClientSocket::usingSSL() {
  return this->s_SSLCtx != nullptr;
}

string ClientSocket::getPrefix() {
  return inet_ntoa(this->s_SocketAddr.sin_addr);
}

ClientSocket &ClientSocket::connectAsClient(const char *host, const int32_t port) {
  auto &fd = this->s_SocketFD;
  auto &addr = this->s_SocketAddr;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
  }

  memset(&addr, 0, sizeof (addr));
  addr.sin_addr.s_addr = inet_addr(host);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (connect(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof (addr)) == -1) {
    throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
  }

  return *this;
}

ClientSocket &ClientSocket::upgradeAsClient() {
  auto *&ssl = this->s_SSL;
  auto *&ctx = this->s_SSLCtx;
  auto &fd = this->s_SocketFD;

  assert(("SSLContext does not exist", ctx != nullptr));

  ssl = SSL_new(ctx->p_SSLCtx);
  if (ssl <= 0) {
    throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
  }

  SSL_set_fd(ssl, fd);
  if (SSL_connect(ssl) <= 0) {
    throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
  }

  return *this;
}