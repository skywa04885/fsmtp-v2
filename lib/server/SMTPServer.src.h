#include <iostream>
#include <string>
#include <atomic>
#include <vector>
#include <bitset>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../smtp/Response.src.h"
#include "../networking/SMTPSocket.src.h"
#include "../general/logger.src.h"
#include "../general/macros.src.h"

#pragma once

#define _SERVER_OPT_ENABLE_AUTH 0x1
#define _SERVER_OPT_ENABLE_TLS 0x2
#define _SERVER_OPT_ENABLE_ENCHANCED_STATUS_CODES 0x4
#define _SERVER_OPT_ENABLE_PIPELINING 0x8

using namespace FSMTP;
using namespace SMTP;
using Networking::SMTPSocket;
using Networking::SMTPSocketType;

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