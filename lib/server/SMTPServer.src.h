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

#include <iostream>
#include <string>
#include <atomic>
#include <vector>
#include <stdexcept>
#include <bitset>
#include <mutex>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "../smtp/Response.src.h"
#include "../smtp/Command.src.h"
#include "../networking/SMTPSocket.src.h"
#include "../general/logger.src.h"
#include "../general/macros.src.h"
#include "SMTPServerSession.src.h"
#include "SMTPServerExceptions.src.h"
#include "SMTPAuthentication.src.h"
#include "../parsers/mime.src.h"
#include "../models/LocalDomain.src.h"

#define _SERVER_OPT_ENABLE_AUTH 0x1
#define _SERVER_OPT_ENABLE_TLS 0x2
#define _SERVER_OPT_ENABLE_ENCHANCED_STATUS_CODES 0x4
#define _SERVER_OPT_ENABLE_PIPELINING 0x8

#define _SERVER_OPT_ENABLE_HELP 0xF

using namespace FSMTP::Parsers;
using namespace FSMTP;
using namespace FSMTP::SMTP;
using namespace FSMTP::Networking;
using namespace FSMTP::Models;
using namespace FSMTP::Networking;

extern std::vector<FullEmail> _emailStorageQueue;
extern std::mutex _emailStorageMutex;

namespace FSMTP::Server
{
	class SMTPServer
	{
	public:
		/**
		 * Default constructor for the SMTPServer
		 *
		 * @Param {const int32_t} port
		 * @Param {const bool} s_UseESMTP
		 * @Param {const int32_t} s_Opts
		 * @Param {const int32_t} s_RedisPort
		 * @Param {ocnst std::string &} s_RedisHost
		 */
		SMTPServer(
			const int32_t port,
			const bool s_UseESMTP,
			const int32_t s_Opts,
			const int32_t s_RedisPort,
			const std::string &s_RedisHost
		);

		/**
		 * Gets the socket
		 *
		 * @Param {void}
		 * @Return {SMTPSocket &} socket
		 */
		SMTPSocket &getSocket(void);

		/**
		 * The method which gets called when an client
		 * - has connected
		 *
		 * @Param {struct sockaddr_in *} sockaddr
		 * @Param {int32_t} fd
		 * @Param {void *} u
		 * @Return {void}
		 */
		static void onClientSync(struct sockaddr_in *sockaddr, int32_t fd, void *u);
		
		/**
		 * Closes the SMTP Server
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void shutdownServer(void);

		std::vector<SMTPServiceFunction> s_Services;
		bool s_UseESMTP;
	private:
		SMTPSocket s_Socket;
		std::atomic<bool> s_IsRunning;
		std::atomic<bool> s_ShouldBeRunning;
		Logger s_Logger;
		int32_t s_Opts;
		int32_t s_RedisPort;
		std::string s_RedisHost;
	};
}
