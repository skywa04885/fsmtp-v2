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
	SMTPClientSocket::SMTPClientSocket(
		const char *hostname,
		const int32_t s_SocketPort
	):
		s_SocketPort(s_SocketPort), s_UseSSL(false)
	{
		// Creates the socket and throws error if anything goes wrong
		this->s_SocketFD = socket(AF_INET, SOCK_STREAM, 0);
		if (this->s_SocketFD < 0)
			throw std::runtime_error("Could not create socket !");

		// Configures the address, so we can connect
		// - to the host when wanted
		this->s_SockAddr.sin_port = htons(this->s_SocketPort);
		inet_aton(hostname, &this->s_SockAddr.sin_addr);
		this->s_SockAddr.sin_family = AF_INET;

		// Sets the connect timeout
		int synRetries = 1;
		setsockopt(this->s_SocketFD, IPPROTO_TCP, TCP_SYNCNT, &synRetries, sizeof (synRetries));
	}

	/**
	 * Starts connecting to the server ( client only )
	 *
	 * @Param void
	 * @Return void
	 */
	void SMTPClientSocket::startConnecting(void)
	{
		// Tries to connect and throws error if something went wrong
		int32_t rc = connect(
			this->s_SocketFD,
			reinterpret_cast<struct sockaddr *>(&this->s_SockAddr),
			sizeof (this->s_SockAddr)
		);
		if (rc < 0)
			throw SMTPConnectError("Could not connect to the server");
	}

	/**
	 * Receives data until an newline occurs
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	std::string SMTPClientSocket::readUntillNewline(void)
	{
		char buffer[1024];
		int32_t rc;
		std::size_t index;
		std::string res;

		// Searches for the newline inside of the buffer
		// - so we can later get the exact message
		for (;;)
		{
			rc = recv(this->s_SocketFD, buffer, 1024, MSG_PEEK);
			if (rc < 0)
				throw SMTPTransmissionError("Could not peek data");

			bool newlineFound = false;
			for (index = 0; index < rc; index++)
			{
				if (buffer[index] == '\n')
				{
					newlineFound = true;
					break;
				}
			}
			if (newlineFound) break;
		}


		// Reads data from the socket into the buffer
		// - then we clear the buffer and append it to
		// - the result
		rc = recv(this->s_SocketFD, buffer, ++index, 0);
		if (rc < 0)
			throw SMTPTransmissionError("Could not receive data");
		res += std::string(buffer, rc);

		// Clears the buffer
		memset(buffer, 0, 1024);

		return res.substr(0, res.size() - 2);
	}

	/**
	 * Writes an command to the client
	 *
	 * @Param {ClientCommandType} type
	 * @Param {const std::vector<std::string> &} args
	 * @Return {void}
	 */
	void SMTPClientSocket::writeCommand(
		ClientCommandType type, 
		const std::vector<std::string> &args
	)
	{
		ClientCommand command(type, args);
		std::string message = command.build();
		this->sendMessage(message);
	}
	
	/**
	 * Sends an string
	 * 
	 * @Param {const std::string &} message
	 * @Return {void}
	 */
	void SMTPClientSocket::sendMessage(const std::string &message)
	{
		int32_t rc;

		// Checks if we need to use ssl or not,
		// - then we send the message and check for errors
		if (this->s_UseSSL)
		{
			rc = SSL_write(this->s_SSL, message.c_str(), message.size());
			if (rc <= 0)
			{
				BIO *bio = BIO_new(BIO_s_mem());
				ERR_print_errors(bio);
				char *err = nullptr;
				std::size_t errLen = BIO_get_mem_data(bio, err);
				std::string error(err, errLen);
				BIO_free(bio);
				throw std::runtime_error("SSL_write() failed: " + error);
			}
		} else
		{
			rc = send(this->s_SocketFD, message.c_str(), message.size(), 0);
			if (rc < 0)
			{
				std::string error = "send() failed: ";
				error += strerror(errno);
				throw SMTPTransmissionError(error);
			}
		}
	}

	/**
	 * Upgrades the socket to an SSL socket
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void SMTPClientSocket::upgradeToSSL(void)
	{
		int32_t rc;

		// Creates the SSL context, and throws error
		// - if something goes wrong
		const SSL_METHOD *method = SSLv23_client_method();
		this->s_SSLCtx = SSL_CTX_new(method);
		if (!this->s_SSLCtx)
			throw SMTPSSLError("Could not initialize the SSL context");

		// Creates the SSL struct, and accepts the SSL connection
		// - with the server, if error just throw it
		this->s_SSL = SSL_new(this->s_SSLCtx);
		SSL_set_fd(this->s_SSL, this->s_SocketFD);

		rc = SSL_connect(this->s_SSL);
		if (rc <= 0)
			throw SMTPSSLError("Could not accept the SSL client");

		this->s_UseSSL = true;
	}

	/**
	 * Destructor override, frees the memory and closes
	 * - open connections
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	SMTPClientSocket::~SMTPClientSocket()
	{
		// Closes the socket, and frees ssl memory
		// - if we're using ssl
		shutdown(this->s_SocketFD, SHUT_RDWR);
		if (this->s_UseSSL)
		{
			SSL_shutdown(this->s_SSL);
			SSL_free(this->s_SSL);
			SSL_CTX_free(this->s_SSLCtx);
		}
	}

	/**
	 * The constructor for the SMTPSocket class
	 *
	 * @Param {SMTPSocketType &} s_SocketType
	 * @Param {int32_t &} s_SocketPort
	 * @Return void
	 */
	SMTPSocket::SMTPSocket(
		const int32_t &s_SocketPort
	): s_SocketPort(s_SocketPort)
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
		int32_t rc;

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

			// Sets the socket timeout so some clients
			// - will not keep the server blocked
			struct timeval timeout;
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;

			rc = setsockopt(clientSockFD, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&timeout), sizeof (timeout));
			if (rc < 0)
			{
				std::cerr << "setsockopt() failed: " << strerror(rc) << std::endl;
				continue;
			}
			rc = setsockopt(clientSockFD, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&timeout), sizeof (timeout));
			if (rc < 0)
			{
				std::cerr << "setsockopt() failed: " << strerror(rc) << std::endl;
				continue;
			}

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
	void SMTPSocket::sendString(int32_t &sfd, SSL *ssl, const bool &useSSL, std::string &data)
	{
		std::size_t rc;
		if (useSSL)
		{
			rc = SSL_write(ssl, data.c_str(), data.length());

			if (rc <= 0)
			{
				BIO *bio = BIO_new(BIO_s_mem());
				ERR_print_errors(bio);
				char *err = nullptr;
				std::size_t errLen = BIO_get_mem_data(bio, err);
				std::string error(err, errLen);
				BIO_free(bio);
				throw std::runtime_error("SSL_write() failed: " + error);
			}
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
	void SMTPSocket::receiveString(
		int32_t &sfd,
		SSL *ssl,
		const bool &useSSL,
		const bool &bigData,
		std::string &ret
	)
	{
		uint8_t *buffer = reinterpret_cast<uint8_t *>(malloc(sizeof (uint8_t) * _SMTP_RECEIVE_BUFFER_SIZE));
		std::size_t rc, bufferPos = 0, bufferSize = _SMTP_RECEIVE_BUFFER_SIZE;

		// Creates the read loop and resizes the buffer if
		// the buffer size is exceeded
		while (true)
		{
			// Checks if we need to allocate more memory
			// - since it will otherwise use unallocated memory
			// - which will just break everything
			if (bufferPos + _SMTP_RECEIVE_BUFFER_SIZE >= bufferSize)
			{
				bufferSize += _SMTP_RECEIVE_BUFFER_SIZE;
				buffer = reinterpret_cast<uint8_t *>(realloc(buffer, bufferSize));
			}

			// Receives the data and then increments the buffer position
			// - after that we check if there is a <CR><LF> in the message
			if (useSSL)
				rc = SSL_read(ssl, reinterpret_cast<void *>(&buffer[bufferPos]), _SMTP_RECEIVE_BUFFER_SIZE);
			else
				rc = recv(sfd, reinterpret_cast<char *>(&buffer[bufferPos]), _SMTP_RECEIVE_BUFFER_SIZE, 0x0);
			if (rc <= 0)
			{
				free(buffer);

				if (useSSL)
				{
					BIO *bio = BIO_new(BIO_s_mem());
					ERR_print_errors(bio);
					char *err = nullptr;
					std::size_t errLen = BIO_get_mem_data(bio, err);
					std::string error(err, errLen);
					BIO_free(bio);
					throw std::runtime_error("SSL_read() failed: " + error);
				} else
				{
					throw std::runtime_error("Could not receive data !");
				}
			}
			bufferPos += rc;

			if (!bigData)
			{
	      if (bufferPos >= 2) if (
					memcmp(&buffer[bufferPos-2], "\r\n", 2) == 0
				) break;
			} else
			{
				if (bufferPos >= 5) if (
					memcmp(&buffer[bufferPos-5], "\r\n.\r\n", 5) == 0
				) break;
			}
		}

		// Creates an deep copy of the buffer into an string
		// - after that we free the buffer which is now trash
		if (!bigData) ret = std::string(reinterpret_cast<char *>(buffer), bufferPos - 2);
		else ret = std::string(reinterpret_cast<char *>(buffer), bufferPos - 5);
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
	void SMTPSocket::upgradeToSSL(
		int32_t *sfd,
		SSL **ssl, 
		SSL_CTX **sslCtx
	)
	{
		int32_t rc;

		// Creates the SSL context and if something goes
		// - wrong we throw an error
		const SSL_METHOD *sslMethod = SSLv23_server_method();
		*sslCtx = SSL_CTX_new(sslMethod);
		if (!sslCtx)
			throw std::runtime_error("Could not create context !");

		// Configures the SSL context, with the keys etcetera
		SSL_CTX_set_ecdh_auto(sslCtx, 1);
		SSL_CTX_set_default_passwd_cb(*sslCtx, &SMTPSocket::readSSLPassphrase);
		
		rc = SSL_CTX_use_certificate_file(*sslCtx, _SMTP_SSL_CERT_PATH, SSL_FILETYPE_PEM);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw std::runtime_error("Could not read cert !");
		}

		rc = SSL_CTX_use_PrivateKey_file(*sslCtx, _SMTP_SSL_KEY_PATH, SSL_FILETYPE_PEM);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw std::runtime_error("Could not read private key !");
		}

		// Creates the ssl element and binds it with the socket,
		// - after that we accept the connection with the client
		*ssl = SSL_new(*sslCtx);
		SSL_set_fd(*ssl, *sfd);

		rc = SSL_accept(*ssl);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw std::runtime_error("Could not accept SSL connection !");
		}
	}
}