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
#include <netinet/in.h>
#include <memory.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "../general/macros.src.h"

#ifndef _SOCKET_MAX_IN_QUEUE
#define _SOCKET_MAX_IN_QUEUE 40
#endif

#ifndef _SOCKET_THREAD_SHUTDOWN_DELAY
#define _SOCKET_THREAD_SHUTDOWN_DELAY 120
#endif

namespace FSMTP::Networking
{
	typedef enum : uint8_t
	{
		SST_CLIENT = 0,
		SST_SERVER
	} SMTPSocketType;

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
		SMTPSocket(const SMTPSocketType &s_SocketType,
			const int32_t &s_SocketPort);

		/**
		 * Starts listening the socket ( server only )
		 *
		 * @Param void
		 * @Return void
		 */
		void startListening(void);

		/**
		 * Starts connecting to the server ( client only )
		 *
		 * @Param void
		 * @Return void
		 */
		void startConnecting(void);

		/**
		 * Static method which sends an string to an client
		 * 
		 * @Param {int32_t &} sfd
		 * @Param {bool &} ssl
		 * @Param {std::string &} data
		 * @Return void
		 */
		static void sendString(int32_t &sfd, const bool& ssl, std::string& data);
		
		/**
		 * Static method which receives an string fron an socket
		 * 
		 * @Param {int32_t &} sfd
		 * @Param {bool &} ssl
		 * @Param {bool &} bigData - If we are getting the DATA section of message
		 * @Param {std::string &} ret
		 * @Return void
		 */
		static void receiveString(int32_t &sfd, const bool& ssl, const bool &bigData, std::string& ret);

		/**
		 * Static method to upgrade an existing socket to an SSL socket
		 *
		 * @Param {int32_t &} sfd
		 * @Param {SSL *} ssl
		 * @Param {SSL_CTX *} sslCtx
		 * @Return void
		 */
		static void upgradeToSSL(int32_t &sfd, SSL *ssl, SSL_CTX *sslCtx);

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
		SMTPSocketType s_SocketType;
	};
}