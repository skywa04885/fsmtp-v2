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
#include <bitset>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../smtp/Response.src.h"
#include "../smtp/Command.src.h"
#include "../networking/SMTPSocket.src.h"
#include "../general/logger.src.h"
#include "../general/macros.src.h"
#include "SMTPActions.src.h"

#define _SERVER_OPT_ENABLE_AUTH 0x1
#define _SERVER_OPT_ENABLE_TLS 0x2
#define _SERVER_OPT_ENABLE_ENCHANCED_STATUS_CODES 0x4
#define _SERVER_OPT_ENABLE_PIPELINING 0x8

#define _SERVER_OPT_ENABLE_HELP 0xF

using namespace FSMTP;
using namespace FSMTP::SMTP;
using namespace FSMTP::Networking;
using namespace FSMTP::Networking;

namespace FSMTP::Server
{
	class SMTPServer
	{
	public:
		SMTPServer(const int32_t &port, const bool& s_UseESMTP, const int32_t &s_Opts);

		SMTPSocket &getSocket(void);

		static void onClientSync(std::shared_ptr<struct sockaddr_in> sockaddr, int32_t fd, void *u);
		void shutdown(void);

		std::vector<SMTPServiceFunction> s_Services;

		bool s_UseESMTP;
	private:
		SMTPSocket s_Socket;
		std::atomic<bool> s_IsRunning;
		std::atomic<bool> s_ShouldBeRunning;
		Logger s_Logger;
		int32_t s_Opts;
	};
}
