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

namespace FSMTP::Sockets {
  ClientSocket::ClientSocket() noexcept:
    s_SSL(nullptr), s_SSLCtx(nullptr), m_Type(Networking::IP::Protocol::Protocol_IPv4)
  {};

  ClientSocket::ClientSocket(Networking::IP::Protocol type) noexcept:
    s_SSL(nullptr), s_SSLCtx(nullptr), m_Type(type)
  {}

  ClientSocket::~ClientSocket() noexcept {
    if (this->s_SSL) {
      SSL_free(this->s_SSL);
    }

    shutdown(this->s_SocketFD, SHUT_RDWR);
    close(this->s_SocketFD);
  }

  ClientSocket &ClientSocket::useSSL(SSLContext *ctx) {
    this->s_SSLCtx = ctx;
    return *this;
  }

  ClientSocket &ClientSocket::upgradeAsServer() {
    SSL *&ssl = this->s_SSL;
    int32_t &fd = this->s_SocketFD;
    auto *ctx = this->s_SSLCtx;

    assert(("SSLContext does not exist", ctx != nullptr));

    // Creates the SSL struct, with the SSL context
    ssl = SSL_new(ctx->p_SSLCtx);
    if (!ssl) throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));

    // Sets the file descriptor and accepts the SSL connection
    //  if this fails throw ssl error
    SSL_set_fd(ssl, fd);
    if (SSL_accept(ssl) <= 0) throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));

    return *this;
  }

  ClientSocket &ClientSocket::acceptAsServer(const int32_t server) {
    // Gets the socket address pointer, and the length
    //  sicne we have ipv6 and ipv4 we switch
    struct sockaddr *addr = nullptr;
    socklen_t len = 0;
    switch (this->m_Type) {
      case Networking::IP::Protocol::Protocol_IPv4:
        addr = reinterpret_cast<struct sockaddr *>(this->getAddress().ipv4);
        len = sizeof (struct sockaddr_in);
        break;
      case Networking::IP::Protocol::Protocol_IPv6:
        addr = reinterpret_cast<struct sockaddr *>(this->getAddress().ipv6);
        len = sizeof (struct sockaddr_in6);
        break;
    }

    // Loops untill we've accepted an socket, if this fails
    //  we just proceed and try again
    for (;;) {
      if ((this->s_SocketFD = accept(server, addr, &len)) == -1) {
        this_thread::sleep_for(milliseconds(1));
        close(this->s_SocketFD);
        continue;
      } else break;
    }

    // Checks the real type, by checking if ::ffff: is inside
    //  of the IPv6 address, if so we set the real type to IPv4
    string prefix = this->getPrefix(false);
    if (prefix.substr(0, 7) == "::ffff:") this->m_RealType = Networking::IP::Protocol::Protocol_IPv4;
    else if (this->m_Type == Networking::IP::Protocol::Protocol_IPv6) this->m_RealType = Networking::IP::Protocol::Protocol_IPv6;
    else this->m_RealType = Networking::IP::Protocol::Protocol_IPv4;

    return *this;
  }

  int32_t ClientSocket::write(const char *msg, const size_t len) {
    int32_t rc;

    // cout << "SockWrite: " << string(msg, len) << endl;

    if (this->s_SSLCtx) {
      if ((rc = SSL_write(this->s_SSL, msg, len)) <= 0) {
        throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
      }
    } else {
      if ((rc = send(this->s_SocketFD, msg, len, 0)) == -1) {
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

    // cout << "SockRead: " << string(buffer, bufferSize) << endl;

    if (this->s_SSLCtx) {
      if ((rc = SSL_read(this->s_SSL, buffer, bufferSize)) <= 0) {
        throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
      }
    } else {
      if ((rc = recv(this->s_SocketFD, buffer, bufferSize, 0)) < 0) {
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
      if ((rc = recv(this->s_SocketFD, buffer, bufferSize, MSG_PEEK)) < 0) {
        throw runtime_error(EXCEPT_DEBUG(strerror(errno)));
      }
    }

    return rc;
  }

  string ClientSocket::readToDelim(const char *delim, size_t maxSize) {
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
      if (readLen <= 0) {
        this_thread::sleep_for(milliseconds(40));
      }

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

      // Prevents too long loop
      if (result.length() > maxSize) {
        delete[] buffer;
        throw SocketReadLimit("Buffer exceeded max size");
      }
    }

    delete[] buffer;
    return result.substr(0, result.length() - delimSize);
  }

  bool ClientSocket::usingSSL() {
    return this->s_SSLCtx != nullptr;
  }

  string ClientSocket::getPrefix(bool clean) {
    string result;

    switch (this->m_Type) {
      case Networking::IP::Protocol::Protocol_IPv4: {
        char buffer[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &this->m_IPv4Addr.sin_addr, buffer, sizeof (buffer)) == nullptr)
          throw runtime_error(EXCEPT_DEBUG(string("inet_ntop() failed: ") + strerror(errno)));
        
        result = buffer;
        break;
      }
      case Networking::IP::Protocol::Protocol_IPv6: {
        char buffer[INET6_ADDRSTRLEN];
        if (inet_ntop(AF_INET6, &this->m_IPv6Addr.sin6_addr, buffer, sizeof (buffer)) == nullptr)
          throw runtime_error(EXCEPT_DEBUG(string("inet_ntop() failed: ") + strerror(errno)));
        
        result = buffer;
        if (clean && this->m_RealType == Networking::IP::Protocol::Protocol_IPv4) result = result.substr(7);
        break;
      }
    }

    return result;
  }

  string ClientSocket::getReverseLookup() {
    switch (this->m_Type) {
      case Networking::IP::Protocol::Protocol_IPv4:
        return DNS::getHostnameByAddress<struct sockaddr_in>(&this->m_IPv4Addr); break;
      case Networking::IP::Protocol::Protocol_IPv6:
        return DNS::getHostnameByAddress<struct sockaddr_in6>(&this->m_IPv6Addr); break;
    }
  }

  string ClientSocket::getString() {
    string result;

    result = this->getReverseLookup();
    result += " [" + this->getPrefix() + "]:" + to_string(this->getPort());

    return result;
  }

  Networking::IP::Protocol ClientSocket::getRealProtocol()
  { return this->m_RealType; }

  __SockAddrReturnP ClientSocket::getAddress() {
    __SockAddrReturnP ret;
    switch (this->m_Type) {
      case Networking::IP::Protocol::Protocol_IPv4: ret.ipv4 = &this->m_IPv4Addr; break;
      case Networking::IP::Protocol::Protocol_IPv6: ret.ipv6 = &this->m_IPv6Addr; break;
    }
    return ret;
  }

  ClientSocket &ClientSocket::connectAsClient(const char *host, const int32_t port) {
    auto &fd = this->s_SocketFD;

    // Creates the socket FD with either AF_INET or AF_INET6
    int32_t type = 0;
    switch (this->m_Type) {
      case Networking::IP::Protocol::Protocol_IPv4: type = AF_INET; break;
      case Networking::IP::Protocol::Protocol_IPv6: type = AF_INET6; break;
    }
    fd = socket(type, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1)
      throw runtime_error(EXCEPT_DEBUG(
        string("Could not create socket: ") + strerror(errno)));

    // Parses the raw address string onto the socket structure
    //  of the current socket instance
    switch (this->m_Type) {
      case Networking::IP::Protocol::Protocol_IPv4:
        memset(&this->m_IPv4Addr, 0, sizeof (struct sockaddr_in));
        inet_pton(AF_INET, host, &this->m_IPv4Addr.sin_addr);
        this->m_IPv4Addr.sin_family = AF_INET;
        this->m_IPv4Addr.sin_port = htons(port);
        break;
      case Networking::IP::Protocol::Protocol_IPv6:
        memset(&this->m_IPv6Addr, 0, sizeof (struct sockaddr_in6));
        inet_pton(AF_INET6, host, &this->m_IPv6Addr.sin6_addr);
        this->m_IPv6Addr.sin6_family = AF_INET6;
        this->m_IPv6Addr.sin6_port = htons(port);
        break;
      default: throw runtime_error(EXCEPT_DEBUG("Invalid address enum"));
    }

    // Connects to the server and throws an error if this fails
    struct sockaddr *addr = nullptr;
    socklen_t len = 0;
    switch (this->m_Type){
      case Networking::IP::Protocol::Protocol_IPv4:
        addr = reinterpret_cast<struct sockaddr *>(&this->m_IPv4Addr);
        len = sizeof(struct sockaddr_in);
        break;
      case Networking::IP::Protocol::Protocol_IPv6:
        addr = reinterpret_cast<struct sockaddr *>(&this->m_IPv6Addr);
        len = sizeof(struct sockaddr_in6);
        break;
    }
    if (connect(fd, addr, len) == -1)
      throw runtime_error(EXCEPT_DEBUG(
        string("Failed to connect to server: ") + strerror(errno)));

    return *this;
  }

  ClientSocket &ClientSocket::upgradeAsClient() {
    auto *&ssl = this->s_SSL;
    auto *&ctx = this->s_SSLCtx;
    auto &fd = this->s_SocketFD;

    assert(("SSLContext does not exist", ctx != nullptr));

    // Creates the SSL struct with the ssl context
    //  and throws an error of it fails
    ssl = SSL_new(ctx->p_SSLCtx);
    if (!ssl) throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));

    // Sets the file descriptor and initializes the SSL
    //  connection to the server
    SSL_set_fd(ssl, fd);
    if (SSL_connect(ssl) <= 0) throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));

    return *this;
  }

  ClientSocket &ClientSocket::timeout(const int32_t s) {
    auto &sock = this->s_SocketFD;
    
    struct timeval timeo;
    timeo.tv_sec = s;
    timeo.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&timeo), sizeof(timeo)) < 0) {
      throw runtime_error(strerror(errno));
    } else if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&timeo), sizeof(timeo)) < 0) {
      throw runtime_error(strerror(errno));
    }

    return *this;
  }

  int32_t ClientSocket::getPort() {
    switch (this->m_Type) {
      case  Networking::IP::Protocol::Protocol_IPv4: return ntohs(this->m_IPv4Addr.sin_port);
      case  Networking::IP::Protocol::Protocol_IPv6: return ntohs(this->m_IPv6Addr.sin6_port);
    }
  }
}