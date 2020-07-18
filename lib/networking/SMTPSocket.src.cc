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
	// ================================================
	//
	// =>
	//		SMTPClientSocket methods
	// =>
	//
	// ================================================

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
			if (this->s_UseSSL)
			{
				rc = SSL_peek(this->s_SSL, buffer, 1024);
				if (rc <= 0)
					throw SMTPTransmissionError("Could not peek data (TLS)");
			} else
			{
				rc = recv(this->s_SocketFD, buffer, 1024, MSG_PEEK);
				if (rc <= 0)
					throw SMTPTransmissionError("Could not peek data");
			}

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
		if (this->s_UseSSL)
		{
			rc = SSL_read(this->s_SSL, buffer, ++index);
			if (rc <= 0)
				throw SMTPTransmissionError("Could not receive data (TLS)");
		} else
		{
			rc = recv(this->s_SocketFD, buffer, ++index, 0);
			if (rc < 0)
				throw SMTPTransmissionError("Could not receive data");
		}
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
		if (this->s_UseSSL)
		{
			// SSL_shutdown(this->s_SSL);
			SSL_free(this->s_SSL);
			SSL_CTX_free(this->s_SSLCtx);
		}
	}

	// ================================================
	//
	// =>
	//		SMTPSocket methods
	// =>
	//
	// ================================================

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

	// ================================================
	//
	// =>
	//		SMTPServerClientSocket methods
	// =>
	//
	// ================================================

	/**
	 * Default empty constructor for the SMTPServerClientSocket
	 *
	 * @Param {Logger &} s_Logger
	 * @Return {void}
	 */
	SMTPServerClientSocket::SMTPServerClientSocket(Logger &s_Logger):
		s_Logger(s_Logger)
	{}

	/**
	 * The constructor which adapts existing socket
	 *
	 * @Param {const int32_t} s_SocketFD
	 * @Param {struct sockaddr_in} s_SockAddr
	 * @Param {Logger &} s_Logger
	 * @Return {void}
	 */
	SMTPServerClientSocket::SMTPServerClientSocket(
		const int32_t s_SocketFD,
		struct sockaddr_in s_SockAddr,
		Logger &s_Logger
	):
		s_SocketFD(s_SocketFD), s_SockAddr(s_SockAddr), s_UseSSL(false),
		s_Logger(s_Logger)
	{}

	/**
	 * Constructor override, frees the memory and closes
	 * - the ongoing connection
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	SMTPServerClientSocket::~SMTPServerClientSocket(void)
	{
		this->s_Logger << WARN << "Verbinding wordt gesloten!" << ENDL << CLASSIC;

		// Closes the socket, and checks if we should free
		// - the openssl context
		shutdown(this->s_SocketFD, SHUT_RDWR);
		if (this->s_UseSSL)
		{
			SSL_free(this->s_SSL);
			SSL_CTX_free(this->s_SSLCtx);
		}
	}

	/**
	 * Sends an message to the client
	 *
	 * @Param {const std::string &} raw
	 * @Return {void}
	 */
	void SMTPServerClientSocket::sendMessage(const std::string &raw)
	{
		int32_t rc;

		// Prints the debug message, if enabled
		DEBUG_ONLY(this->s_Logger << DEBUG << "S->" << raw.substr(0, raw.size() - 2) << ENDL << CLASSIC);

		// Checks if we need to use ssl or not,
		// - then we send the message and check for errors
		if (this->s_UseSSL)
		{
			rc = SSL_write(this->s_SSL, raw.c_str(), raw.size());
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
			rc = send(this->s_SocketFD, raw.c_str(), raw.size(), 0);
			if (rc < 0)
			{
				std::string error = "send() failed: ";
				error += strerror(errno);
				throw SMTPTransmissionError(error);
			}
		}
	}

	/**
	 * Reads data untill an newline is received
	 *
	 * @Param {const bool} big
	 * @Return {std::string}
	 */
	std::string SMTPServerClientSocket::readUntillNewline(const bool big)
	{
		int32_t rc;
		char *buffer = new char[128];
		std::string res;

		for (;;)
		{
			if (this->s_UseSSL)
			{
				rc = SSL_read(this->s_SSL, buffer, 128);
				if (rc <= 0)
				{
					delete[] buffer;
					throw std::runtime_error("Could not read data (ssl)");
				}
			} else
			{
				rc = recv(this->s_SocketFD, buffer, 128, 0);
				if (rc < 0)
				{
					delete[] buffer;
					throw std::runtime_error("Could not read data");
				}
			}

			res += std::string(buffer, rc);
			memset(buffer, 0, 128);

			if (big) 
			{
				if (res.size() > 5 && res.substr(res.size() - 5) == "\r\n.\r\n")
				{
					break;
				}
			}
			else if (res.size() >= 2 && res.substr(res.size() - 2) == "\r\n")
			{
				break;
			}
		}

		// Prints the debug message, if enabled
		#ifdef _SMTP_DEBUG
		if (!big) this->s_Logger << DEBUG << "C->" << res.substr(0, res.size() - 2) << ENDL << CLASSIC;
		else this->s_Logger << DEBUG << "C->[BIG DATA]" << ENDL << CLASSIC;
		#endif

		delete[] buffer;
		if (big) return res.substr(0, res.size() - 5);
		else return res.substr(0, res.size() - 2);
	}

	/**
	 * Sends an response to the client
	 * @Param {const SMTPResponseType} c_Type
	 * @Return {void}
	 */
	void SMTPServerClientSocket::sendResponse(const SMTPResponseType c_Type)
	{
		ServerResponse response(c_Type);
		std::string message = response.build();
		this->sendMessage(message);
	}

	/**
	 * Sends an response to the client
	 *
	 * @Param {const SMTPResponseType} c_Type
	 * @Param {const std::string &} c_Message
	 * @Param {void *} c_U
	 * @Param {const std::vector<SMTPServiceFunction *} c_Services
	 * @Return {void}
	 */
	void SMTPServerClientSocket::sendResponse(
		const SMTPResponseType c_Type,
		const std::string &c_Message,
		void *c_U,
		std::vector<SMTPServiceFunction> *c_Services
	)
	{
		ServerResponse response(c_Type, c_Message, c_U, c_Services);
		std::string message = response.build();
		this->sendMessage(message);
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
	int SMTPServerClientSocket::readSSLPassphrase(char *buffer, int size, int rwflag, void *u)
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
	 * Upgrades connection to ssl
	 *
	 * @Param {void}
	 * @Return void
	 */
	void SMTPServerClientSocket::upgrade(void)
	{
		int32_t rc;

		this->s_Logger << "Verbinding wordt beveiligd ..." << ENDL;

		// Creates the SSL context and if something goes
		// - wrong we throw an error
		const SSL_METHOD *sslMethod = SSLv23_server_method();
		this->s_SSLCtx = SSL_CTX_new(sslMethod);
		if (!this->s_SSLCtx)
			throw std::runtime_error("Could not create context !");

		// Configures the SSL context, with the keys etcetera
		SSL_CTX_set_ecdh_auto(this->s_SSLCtx, 1);
		SSL_CTX_set_default_passwd_cb(this->s_SSLCtx, &SMTPServerClientSocket::readSSLPassphrase);
		
		rc = SSL_CTX_use_certificate_file(this->s_SSLCtx, _SMTP_SSL_CERT_PATH, SSL_FILETYPE_PEM);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw std::runtime_error("Could not read cert !");
		}

		rc = SSL_CTX_use_PrivateKey_file(this->s_SSLCtx, _SMTP_SSL_KEY_PATH, SSL_FILETYPE_PEM);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw std::runtime_error("Could not read private key !");
		}

		// Creates the ssl element and binds it with the socket,
		// - after that we accept the connection with the client
		this->s_SSL = SSL_new(this->s_SSLCtx);
		SSL_set_fd(this->s_SSL, this->s_SocketFD);

		rc = SSL_accept(this->s_SSL);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw std::runtime_error("Could not accept SSL connection !");
		}

		// Sets useSSL to true
		this->s_UseSSL = true;
		this->s_Logger << "Verbinding succesvol beveiligd!" << ENDL;
	}
}