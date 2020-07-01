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

#include "SMTPSocket.src.h"

namespace FSMTP::Networking
{
	/**
	 * The constructor for the SMTPSocket class
	 *
	 * @Param {SMTPSocketType &} s_SocketType
	 * @Param {int32_t &} s_SocketPort
	 * @Return void
	 */
	SMTPSocket::SMTPSocket(
		const SMTPSocketType &s_SocketType,
		const int32_t &s_SocketPort
	): s_SocketType(s_SocketType), s_SocketPort(s_SocketPort)
	{
		if (s_SocketType == SMTPSocketType::SST_CLIENT)
		{

		} else if (s_SocketType == SMTPSocketType::SST_SERVER)
		{
			int32_t rc;

			// Zeros the memory inside the socket address
			// - after that we configure the socket stuff
			memset(&this->s_SockAddr, 0x0, sizeof (sockaddr_in));
			this->s_SockAddr.sin_addr.s_addr = INADDR_ANY;
			this->s_SockAddr.sin_family = AF_INET;
			this->s_SockAddr.sin_port = htons(s_SocketPort);

			// Creates the socket and checks if any error
			// - occured, if so we throw an runtime error
			this->s_SocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (this->s_SocketFD < 0)
			{
				std::string error = "socket() failed: ";
				error += strerror(errno);
				throw std::runtime_error(error);
			}

			// Sets some socket options, one of them is to reuse the address
			// - to prevent some stupid errors
			int32_t opt = 0x1;
			rc = setsockopt(this->s_SocketFD, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&opt), sizeof (opt));
			if (rc < 0)
			{
				std::string error = "setsockopt() failed: ";
				error += strerror(rc);
				throw std::runtime_error(error);
			}

			// Sets the socket into non-blocking mode, so we can handle stuff more
			// - fast and make the loop killable at any time
			opt = 0x1;
			rc = ioctl(this->s_SocketFD, FIONBIO, reinterpret_cast<char *>(&opt));
			if (rc < 0)
			{
				std::string error = "ioctl() failed: ";
				error += strerror(rc);
				throw std::runtime_error(error);
			}

			// Binds the socket
			rc = bind(this->s_SocketFD, reinterpret_cast<struct sockaddr *>(&this->s_SockAddr), sizeof (struct sockaddr));
			if (rc < 0)
			{
				std::string error = "bind() failed: ";
				error += strerror(rc);
				throw std::runtime_error(error);
			}
		} else throw std::runtime_error(
			R"(Invalid socket type supplied, must be either 
			SMTPSocketType::SST_CLIENT or SMTPSocketType::SST_SERVER !)"
		);
	}

	/**
	 * An single instance of an acceptor thread,
	 * - these accept the clients
	 *
	 * @Param {std::atomic<bool> &} run
	 * @Param {std::size_t} delay
	 * @Param {std::atomic<bool> &} running
	 * @Param {std::function<void(params)> &} cb
	 * @param {void *} u
	 * @Return void
	 */
	void SMTPSocket::asyncAcceptorThread(
		std::atomic<bool> &run,
		const std::size_t delay,
		std::atomic<bool> &running,
		const std::function<void(struct sockaddr_in *, int32_t, void *)> &cb,
		void *u
	)
	{
		struct sockaddr_in clientSockAddr;
		int32_t clientSockFD;
		int32_t sockAddrSize = sizeof (struct sockaddr_in);

		// Sets running to true and performs infinite loop
		// - if the run variable is set to false, we will
		// - quit and set running to false
		running = true;
		while (run == true)
		{
			// Sleeps for some time ( before all the code because we may use continue )
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));

			// Accepts the client and checks for any errors, if we have any
			// error we just log it and continue
			clientSockFD = accept(this->s_SocketFD, reinterpret_cast<struct sockaddr *>(&clientSockAddr), reinterpret_cast<socklen_t *>(&sockAddrSize));
			if (clientSockFD < 0)
			{
				if (errno != EWOULDBLOCK)
					std::cout << "accept() failed: " << strerror(clientSockFD) << std::endl;					
				continue;
			}
			std::cout << "Accepted client " << std::endl;

			// Prepares the data, and then creates an new thread for the
			// - callback which will handle the request stuff ;)
			struct sockaddr_in *sAddr = new sockaddr_in;
			memcpy(sAddr, &clientSockAddr, sizeof (struct sockaddr_in));

			std::thread t(cb, sAddr, clientSockFD, u);
			t.detach();
		}
		running = false;
	}

	/**
	 * Starts listening the socket ( server only )
	 *
	 * @Param void
	 * @Return void
	 */
	void SMTPSocket::startListening(void)
	{
		int32_t rc;

		// Listens the socket and checks for errors, if there is any error
		// we will throw it immediately
		rc = listen(this->s_SocketFD, _SOCKET_MAX_IN_QUEUE);
		if (rc < 0)
		{
			std::string error = "listen() failed: ";
			error += strerror(rc);
			throw std::runtime_error(error);
		}
	}

	/**
	 * Starts the client acceptor in sync mode ( The slow and blocking one )
	 *
	 * @Param {std::function<void(params)> &} cb
	 * @Param {std::size_t} delay
	 * @Param {bool &} mult
	 * @Param {std::atomic<bool> &} run
	 * @Param {std::atomic<bool> &} running
	 * @param {void *} u
	 * @Return void
	 */
	void SMTPSocket::startAcceptorSync(
		const std::function<void(struct sockaddr_in *, int32_t, void *)> &cb,
		const std::size_t delay,
		const bool &mult,
		std::atomic<bool> &run,
		std::atomic<bool> &running,
		void *u
	)
	{
		std::thread t(&SMTPSocket::asyncAcceptorThread, this, std::ref(run), delay, std::ref(running), cb, u);
		t.detach();
	}

	/**
	 * Closes the threads
	 *
	 * @Param {std::atomic<bool> &} run
	 * @Param {std::atomic<bool> &} running
	 * @Return void
	 */
	void SMTPSocket::closeThread(std::atomic<bool> &run, std::atomic<bool> &running)
	{
		run = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		while (running) std::this_thread::sleep_for(std::chrono::milliseconds(_SOCKET_THREAD_SHUTDOWN_DELAY));
	}


	/**
	 * Default destructor which closes the socket, so no errors
	 * - will occur
	 *
	 * @Param void
	 * @Return void
	 */
	SMTPSocket::~SMTPSocket()
	{
		// Closes the socket inside of the SMTPSocket instance
		// so the memory will be free and no issues etc
		shutdown(this->s_SocketFD, SHUT_RDWR);
	}

	/**
	 * Static method which sends an string to an client
	 * 
	 * @Param {int32_t &} sfd
	 * @Param {bool &} ssl
	 * @Param {std::string &} data
	 * @Return void
	 */
	void SMTPSocket::sendString(int32_t &sfd, const bool &ssl, std::string &data)
	{
		std::size_t rc;
		if (ssl)
		{
			return;
		}

		// Sends the string and checks for any errors
		rc = send(sfd, data.c_str(), data.length(), 0);
		if (rc < 0)
		{
			std::string error = "send() failed: ";
			error += strerror(rc);
			throw std::runtime_error(error);
		}
	}

	/**
	 * Static method which receives an string fron an socket
	 * 
	 * @Param {int32_t &} sfd
	 * @Param {bool &} ssl
	 * @Param {bool &} bigData - If we are getting the DATA section of message
	 * @Param {std::string &} ret
	 * @Return void
	 */
	void SMTPSocket::receiveString(int32_t &sfd, const bool &ssl, const bool &bigData, std::string &ret)
	{
		uint8_t *buffer = reinterpret_cast<uint8_t *>(malloc(sizeof (uint8_t) * _SMTP_RECEIVE_BUFFER_SIZE));
		std::size_t rc, bufferPos = 0, bufferSize = _SMTP_RECEIVE_BUFFER_SIZE;
		if (ssl)
		{
			return;
		}

		// Creates the read loop and resizes the buffer if
		// the buffer size is exceeded
		while (true)
		{
			// Checks if we need to allocate more memory
			// - since it will otherwise use unallocated memory
			// - which will just break everything
			if (bufferPos >= bufferSize)
			{
				bufferSize += _SMTP_RECEIVE_BUFFER_SIZE;
				buffer = reinterpret_cast<uint8_t *>(realloc(buffer, bufferSize));
			}

			// Receives the data and then increments the buffer position
			// - after that we check if there is a <CR><LF> in the message
			rc = recv(sfd, reinterpret_cast<char *>(&buffer[bufferPos]), _SMTP_RECEIVE_BUFFER_SIZE, 0x0);
			if (rc < 0) continue;
			bufferPos += rc;

      if (bufferPos >= 2) if (
				memcmp(&buffer[bufferPos-2], "\r\n", 2) == 0
			) break;
		}

		// Creates an deep copy of the buffer into an string
		// - after that we free the buffer which is now trash
		ret = std::string(reinterpret_cast<char *>(buffer), bufferPos - 2);
		free(buffer);
	}

	/**
	 * Static single-usage method for reading the OpenSSL keys passphrase
	 *
	 * @Param {char *} buffer
	 * @Param {int} size
	 * @Param {int} rwflag
	 * @param {void *} u
	 * @Return int
	 */
	int SMTPSocket::readSSLPassphrase(char *buffer, int size, int rwflag, void *u)
	{
		// Reads the file and stores it inside the buffer
		// - if something goes wrong we simply throw an error
		FILE *f = fopen(_SMTP_SSL_PASSPHRASE_PATH, "r");
		if (!f)
		{
			std::string error = "fopen() failed: ";
			error += strerror(errno);
			throw std::runtime_error(error);
		}

		fgets(buffer, size, f);
		return strlen(buffer);
	}

	/**
	 * Static method to upgrade an existing socket to an SSL socket
	 *
	 * @Param {int32_t &} sfd
	 * @Param {SSL *} ssl
	 * @Param {SSL_CTX *} sslCtx
	 * @Return void
	 */
	void SMTPSocket::upgradeToSSL(int32_t &sfd, SSL *ssl, SSL_CTX *sslCtx)
	{
		int32_t rc;

		// Creates the SSL context and if something goes
		// - wrong we throw an error
		const SSL_METHOD *sslMethod = SSLv23_server_method();
		sslCtx = SSL_CTX_new(sslMethod);
		if (!sslCtx)
			throw std::runtime_error("Could not create context !");

		// Configures the SSL context, with the keys etcetera
		SSL_CTX_set_ecdh_auto(sslCtx, 0x1);
		SSL_CTX_set_default_passwd_cb(sslCtx, &SMTPSocket::readSSLPassphrase);
		
		rc = SSL_CTX_use_certificate_file(sslCtx, _SMTP_SSL_CERT_PATH, SSL_FILETYPE_PEM);
		if (rc < 0)
		{
			ERR_print_errors_fp(stderr);
			throw std::runtime_error("Could not read cert !");
		}

		rc = SSL_CTX_use_PrivateKey_file(sslCtx, _SMTP_SSL_KEY_PATH, SSL_FILETYPE_PEM);
		if (rc < 0)
		{
			ERR_print_errors_fp(stderr);
			throw std::runtime_error("Could not read private key !");
		}

		// Creates the ssl element and binds it with the socket,
		// - after that we accept the connection with the client
		ssl = SSL_new(sslCtx);
		SSL_set_fd(ssl, sfd);

		rc = SSL_accept(ssl);
		if (rc < 0)
		{
			ERR_print_errors_fp(stderr);
			throw std::runtime_error("Could not accept SSL connection !");
		}
	}
}