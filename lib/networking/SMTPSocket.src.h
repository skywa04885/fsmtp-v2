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

#include <cstdint>
#include <string>
#include <stdexcept>
#include <memory>
#include <functional>
#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <memory.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netinet/tcp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <arpa/inet.h>

#include "../general/macros.src.h"
#include "../smtp/Command.src.h"
#include "../smtp/Response.src.h"

#ifndef _SOCKET_MAX_IN_QUEUE
#define _SOCKET_MAX_IN_QUEUE 40
#endif

#ifndef _SOCKET_THREAD_SHUTDOWN_DELAY
#define _SOCKET_THREAD_SHUTDOWN_DELAY 120
#endif

using namespace FSMTP::SMTP;

namespace FSMTP::Networking
{
	class SMTPTransmissionError : public std::exception
	{
	public:
		SMTPTransmissionError(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what() const throw()
    {
    	return this->e_Message.c_str();
    }
	private:
		std::string e_Message;
	};

	class SMTPConnectError : public std::exception
	{
	public:
		SMTPConnectError(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what() const throw()
    {
    	return this->e_Message.c_str();
    }
	private:
		std::string e_Message;
	};

	class SMTPSSLError : public std::exception
	{
	public:
		SMTPSSLError(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what() const throw()
    {
    	return this->e_Message.c_str();
    }
	private:
		std::string e_Message;
	};

	class SMTPClientSocket
	{
	public:
		/**
		 * Default constructor for the client socket type
		 */
		SMTPClientSocket(
			const char *hostname,
			const int32_t s_SocketPort
		);

		/**
		 * Starts connecting to the server
		 *
		 * @Param void
		 * @Return void
		 */
		void startConnecting(void);

		/**
		 * Receives data until an newline occurs
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		std::string readUntillNewline(void);

		/**
		 * Sends an string
		 * 
		 * @Param {const std::string &} message
		 * @Return {void}
		 */
		void sendMessage(const std::string &message);

		/**
		 * Upgrades the socket to an SSL socket
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void upgradeToSSL(void);

		/**
		 * Writes an command to the client
		 *
		 * @Param {ClientCommandType} type
		 * @Param {const std::vector<std::string> &} args
		 * @Return {void}
		 */
		void writeCommand(
			ClientCommandType type, 
			const std::vector<std::string> &args
		);

		/**
		 * Destructor override, frees the memory and closes
		 * - open connections
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		~SMTPClientSocket();
	private:
		int32_t s_SocketFD;
		int32_t s_SocketPort;
		struct sockaddr_in s_SockAddr;
		bool s_UseSSL;
		SSL *s_SSL;
		SSL_CTX *s_SSLCtx;
	};

	class SMTPSocket
	{
	public:
		/**
		 * The constructor for the SMTPSocket class
		 *
		 * @Param {SMTPSocketType &} s_SocketType
		 * @Param {int32_t &} s_SocketPort
		 * @Return void
		 */
		SMTPSocket(const int32_t &s_SocketPort);

		/**
		 * Starts listening the socket ( server only )
		 *
		 * @Param void
		 * @Return void
		 */
		void startListening(void);

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
		void startAcceptorSync(
			const std::function<void(struct sockaddr_in *, int32_t, void *)> &cb,
			const std::size_t delay,
			const bool &mult,
			std::atomic<bool> &run,
			std::atomic<bool> &running,
			void *u
		);

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
		void asyncAcceptorThread(
			std::atomic<bool> &run,
			const std::size_t delay,
			std::atomic<bool> &running,
			const std::function<void(struct sockaddr_in *, int32_t, void *)> &cb,
			void *u
		);

		/**
		 * Closes the threads
		 *
		 * @Param {std::atomic<bool> &} run
		 * @Param {std::atomic<bool> &} running
		 * @Return void
		 */
		void closeThread(std::atomic<bool> &run, std::atomic<bool> &running);

		/**
		 * Closes the socket, idk when i use this
		 *
		 * @Param void
		 * @Return void
		 */
		void close(void);

		/**
		 * Default destructor which closes the socket, so no errors
		 * - will occur
		 *
		 * @Param void
		 * @Return void
		 */
		~SMTPSocket();
	private:
		int32_t s_SocketFD;
		int32_t s_SocketPort;
		struct sockaddr_in s_SockAddr;
	};

	class SMTPServerClientSocket
	{
	public:
		/**
		 * Default empty constructor for the SMTPServerClientSocket
		 *
		 * @Param {Logger &} c_Logger
		 * @Return {void}
		 */
		explicit SMTPServerClientSocket(Logger &c_Logger);

		/**
		 * The constructor which adapts existing socket
		 *
		 * @Param {const int32_t} s_SocketFD
		 * @Param {struct sockaddr_in} s_SockAddr
		 * @Param {Logger &} s_Logger
		 * @Return {void}
		 */
		SMTPServerClientSocket(
			const int32_t s_SocketFD,
			struct sockaddr_in s_SockAddr,
			Logger &c_Logger
		);

		/**
		 * Constructor override, frees the memory and closes
		 * - the ongoing connection
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		~SMTPServerClientSocket(void);

		/**
		 * Sends an message to the client
		 *
		 * @Param {const std::string &} raw
		 * @Return {void}
		 */
		void sendMessage(const std::string &raw);

		/**
		 * Reads data untill an newline is received
		 *
		 * @Param {const bool} big
		 * @Return {std::string}
		 */
		std::string readUntillNewline(const bool big);

		/**
		 * Sends an response to the client
		 * @Param {const SMTPResponseType} c_Type
		 * @Return {void}
		 */
		void sendResponse(const SMTPResponseType c_Type);

		/**
		 * Sends an response to the client
		 *
		 * @Param {const SMTPResponseType} c_Type
		 * @Param {const std::string &} c_Message
		 * @Param {void *} c_U
		 * @Param {const std::vector<SMTPServiceFunction *} c_Services
		 * @Return {void}
		 */
		void sendResponse(
			const SMTPResponseType c_Type,
			const std::string &c_Message,
			void *c_U,
			std::vector<SMTPServiceFunction> *c_Services
		);

		/**
		 * Upgrades connection to ssl
		 *
		 * @Param {void}
		 * @Return void
		 */
		void upgrade(void);

		/**
		 * Static single-usage method for reading the OpenSSL keys passphrase
		 *
		 * @Param {char *} buffer
		 * @Param {int} size
		 * @Param {int} rwflag
		 * @param {void *} u
		 * @Return int
		 */
		static int readSSLPassphrase(char *buffer, int size, int rwflag, void *u);
	private:
		SSL_CTX *s_SSLCtx;
		SSL *s_SSL;
		bool s_UseSSL;
		int32_t s_SocketFD;
		struct sockaddr_in s_SockAddr;
		Logger &s_Logger;
	};
}