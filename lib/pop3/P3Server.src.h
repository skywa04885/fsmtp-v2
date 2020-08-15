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

#include "P3.src.h"
#include "P3Commands.src.h"
#include "P3Response.src.h"
#include "P3ServerSession.src.h"

using namespace FSMTP::Connections;
using namespace FSMTP::Models;
using namespace FSMTP::Sockets;

namespace FSMTP::POP3
{
	class P3Server
	{
	public:
		P3Server();

		P3Server &listenServer();
		P3Server &connectDatabases();
		P3Server &createContext();
		P3Server &startHandler(const bool newThread);

		bool handleCommand(
			shared_ptr<ClientSocket> client, P3Command &command,
			P3ServerSession &session, Logger &clogger
		);
	private:
		unique_ptr<RedisConnection> s_Redis;
		unique_ptr<CassandraConnection> s_Cassandra;
		unique_ptr<ServerSocket> s_SSLSocket;
		unique_ptr<ServerSocket> s_PlainSocket;
		unique_ptr<SSLContext> s_SSLContext;
		Logger s_Logger;
		std::atomic<bool> s_Run;
		std::atomic<bool> s_Running;
		std::vector<POP3Capability> s_Capabilities;
	};
}
