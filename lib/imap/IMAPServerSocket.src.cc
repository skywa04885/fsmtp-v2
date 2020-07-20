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

#pragma once

#include "IMAPServerSocket.src.h"

namespace FSMTP::IMAP
{
	IMAPServerSocket::IMAPServerSocket(
		const int32_t plainPort,
		const int32_t securePort
	):
		s_Logger("IMAPServSock", LoggerLevel::INFO)
	{
		assertm(
			plainPort != 25 && plainPort != 587 && plainPort != 465,
			"Plain port reserved"
		);
		assertm(
			plainPort != securePort && plainPort != 25 && plainPort != 587 && plainPort != 465,
			"Secure port reserved"
		);

		int32_t rc;
		Logger &logger = this->s_Logger;

		// =================================================
		// Creates the plain text socket
		//
		// The socket which will not be secure by default
		// ==================================================

		// Creates the socket address
		memset(&this->s_PlainSocketAddr, 0, sizeof (struct sockaddr_in));
		this->s_PlainSocketAddr.sin_port = htons(plainPort);
		this->s_PlainSocketAddr.sin_addr.s_addr = INADDR_ANY;
		this->s_PlainSocketAddr.sin_family = AF_INET;

		// Creates the socket
		this->s_PlainSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (this->s_PlainSocketFD < 0)
		{
			std::string error = "socket() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		// Sets the socket reuse property
		int32_t option = 0x1;
		rc = setsockopt(
			this->s_PlainSocketFD,
			SOL_SOCKET,
			SO_REUSEADDR,
			reinterpret_cast<char *>(&option),
			sizeof(option)
		);
		if (rc < 0)
		{
			std::string error = "setsockopt() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		// Makes the socket non blocking
		option = 0x1;
		rc = ioctl(this->s_PlainSocketFD, FIONBIO, reinterpret_cast<char *>(&option));
		if (rc < 0)
		{
			std::string error = "ioctl() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		// Binds the socket
		rc = bind(
			this->s_PlainSocketFD,
			reinterpret_cast<struct sockaddr *>(&this->s_PlainSocketAddr),
			sizeof(struct sockaddr)
		);
		if (rc < 0)
		{
			std::string error = "bind() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		logger << "Plain socket bound on port: " << plainPort << ENDL;

		// =================================================
		// Creates the SSL socket
		//
		// The socket which will be secure by default
		// ==================================================

		// Creates the socket address
		memset(&this->s_SecureSocketAddr, 0, sizeof(struct sockaddr_in));
		this->s_SecureSocketAddr.sin_addr.s_addr = INADDR_ANY;
		this->s_SecureSocketAddr.sin_family = AF_INET;
		this->s_SecureSocketAddr.sin_port = htons(securePort);

		// Creates the socket
		this->s_SecureSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (this->s_SecureSocketFD < 0)
		{
			std::string error = "socket() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		// Sets the socket reuse property
		option = 0x1;
		rc = setsockopt(
			this->s_SecureSocketFD,
			SOL_SOCKET,
			SO_REUSEADDR,
			reinterpret_cast<char *>(&option),
			sizeof(option)
		);
		if (rc < 0)
		{
			std::string error = "setsockopt() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		// Binds the socket
		rc = bind(
			this->s_SecureSocketFD,
			reinterpret_cast<struct sockaddr *>(&this->s_SecureSocketAddr),
			sizeof(struct sockaddr)
		);
		if (rc < 0)
		{
			std::string error = "bind() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		logger << "SSL socket bound on port: " << securePort << ENDL;
	}

	void IMAPServerSocket::startListening(
		std::atomic<bool> *plainRunning,
		std::atomic<bool> *secureRunning,
		std::atomic<bool> *run,
		std::function<void(std::unique_ptr<IMAPClientSocket>, void *)> callback,
		void *u
	)
	{
		Logger &logger = this->s_Logger;
		int32_t rc;

		// Initializes OpenSSL
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();

		// =================================================
		// Listens the SSL socket
		//
		// Starts the listener for the SSL socket
		// ==================================================

		// Listens the plain socket (120 in queue max)
		rc = listen(this->s_SecureSocketFD, 120);
		if (rc < 0)
		{
			std::string error = "listen() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		// Creates the SSL CTX
		this->s_SecureSSLMethod = SSLv23_server_method();
		this->s_SecureSSLCTX = SSL_CTX_new(this->s_SecureSSLMethod);
		if (!this->s_SecureSSLCTX)
		{
			ERR_print_errors_fp(stderr);
			throw SocketInitializationException(EXCEPT_DEBUG("SSL_CTX_new() failed"));
		}

		// Configures the context
		SSL_CTX_set_ecdh_auto(this->s_SecureSSLCTX, 0x1);
		rc = SSL_CTX_use_certificate_file(this->s_SecureSSLCTX, _SMTP_SSL_CERT_PATH, SSL_FILETYPE_PEM);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw SocketInitializationException(EXCEPT_DEBUG("SSL_CTX_use_certificate_file() failed"));	
		}

		rc = SSL_CTX_use_PrivateKey_file(this->s_SecureSSLCTX, _SMTP_SSL_KEY_PATH, SSL_FILETYPE_PEM);
		if (rc < 0)
		{
			ERR_print_errors_fp(stderr);
			throw SocketInitializationException(EXCEPT_DEBUG("SSL_CTX_use_PrivateKey_file() failed"));	
		}

		// Starts the acceptor thread
		std::thread secureAcceptor(
			&IMAPServerSocket::asyncAcceptorThreadSecure,
			this,
			secureRunning,
			run,
			callback,
			u
		);
		secureAcceptor.detach();
		logger << "Secure acceptor detached and running ..." << ENDL;

		// =================================================
		// Listens the plain socket
		//
		// Starts the listener for the plain socket
		// ==================================================

		// Listens the plain socket (120 in queue max)
		rc = listen(this->s_PlainSocketFD, 120);
		if (rc < 0)
		{
			std::string error = "listen() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		// Starts the plain acceptor thread
		std::thread plainAcceptor(
			&IMAPServerSocket::asyncAcceptorThreadPlain,
			this,
			plainRunning,
			run,
			callback,
			u
		);
		plainAcceptor.detach();
		logger << "Plain acceptor detached and running ..." << ENDL;
	}

	void IMAPServerSocket::asyncAcceptorThreadSecure(
		std::atomic<bool> *secureRunning,
		std::atomic<bool> *run,
		std::function<void(std::unique_ptr<IMAPClientSocket>, void *)> callback,
		void *u	
	)
	{
		Logger &logger = this->s_Logger;
		int32_t rc;
		int32_t sockaddrLen = sizeof(struct sockaddr_in);

		// Sets running to true and starts the accept loop
		*secureRunning = true;
		while (*run)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));

			struct sockaddr_in clientAddr;
			SSL *clientSSL = nullptr;
			int32_t clientSocketFD;

			// Accepts the client and checks for errors
			clientSocketFD = accept(
				this->s_SecureSocketFD,
				reinterpret_cast<struct sockaddr *>(&clientAddr),
				reinterpret_cast<socklen_t *>(&sockaddrLen)
			);
			if (clientSocketFD < 0)
			{
				if (errno != EWOULDBLOCK)
					logger << ERROR << "accept() failed: " << strerror(errno) << ENDL << CLASSIC;
				continue;	
			}

			// Prints the accept message
			DEBUG_ONLY(logger << DEBUG << "Client " << inet_ntoa(clientAddr.sin_addr) << " accepted (SSL)" << ENDL << CLASSIC);

			// Creates the SSL, accepts the client
			// - and checks for error
			clientSSL = SSL_new(this->s_SecureSSLCTX);

			SSL_set_fd(clientSSL, clientSocketFD);
			rc = SSL_accept(clientSSL);
			if (rc <= 0)
			{
				SSL_free(clientSSL);
				ERR_print_errors_fp(stderr);
				shutdown(clientSocketFD, SHUT_RDWR);
				logger << ERROR << "SSL_accept() failed" << ENDL << CLASSIC;
				continue;
			}

			// Prints the upgrade message
			DEBUG_ONLY(logger << DEBUG << "Client " << inet_ntoa(clientAddr.sin_addr) << " upgraded" << ENDL << CLASSIC);

			// Creates the new handling thread for the client
			std::thread handler(callback, std::make_unique<IMAPClientSocket>(
				clientAddr, clientSocketFD, clientSSL, this->s_SecureSSLCTX
			), u);
			handler.detach();
		}

		// Sets running to false and waits an short amount of time
		*secureRunning = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
	}

	void IMAPServerSocket::asyncAcceptorThreadPlain(
		std::atomic<bool> *plainRunning,
		std::atomic<bool> *run,
		std::function<void(std::unique_ptr<IMAPClientSocket>, void *)> callback,
		void *u	
	)
	{
		Logger &logger = this->s_Logger;
		int32_t rc;
		int32_t sockaddrLen = sizeof(struct sockaddr_in);

		// Sets running to true and starts the accept loop
		*plainRunning = true;
		while (*run)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));

			struct sockaddr_in clientAddr;
			int32_t clientSocketFD;

			// Accepts the client and checks for errors
			clientSocketFD = accept(
				this->s_PlainSocketFD,
				reinterpret_cast<struct sockaddr *>(&clientAddr),
				reinterpret_cast<socklen_t *>(&sockaddrLen)
			);
			if (clientSocketFD < 0)
			{
				if (errno != EWOULDBLOCK)
					logger << ERROR << "accept() failed: " << strerror(errno) << ENDL << CLASSIC;
				continue;	
			}

			// Prints the accept message
			DEBUG_ONLY(logger << DEBUG << "Client " << inet_ntoa(clientAddr.sin_addr) << " accepted (PLAIN)" << ENDL << CLASSIC);

			// Creates the new handling thread for the client
			std::thread handler(callback, std::make_unique<IMAPClientSocket>(
				clientAddr, clientSocketFD, nullptr, this->s_SecureSSLCTX
			), u);
			handler.detach();
		}

		// Sets running to false and waits an short amount of time
		*plainRunning = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
	}

	/**
	 * Override constructor, destroys the ssl stuff
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	IMAPServerSocket::~IMAPServerSocket(void)
	{
		SSL_CTX_free(this->s_SecureSSLCTX);
	}
}