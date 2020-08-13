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

#include "../../default.h"
#include "../../networking/sockets/ServerSocket.src.h"
#include "../../networking/sockets/SSLContext.src.h"
#include "../../general/Global.src.h"

#include "../Response.src.h"
#include "../Command.src.h"
#include "../../networking/SMTPSocket.src.h"
#include "../../general/Logger.src.h"
#include "../../general/macros.src.h"
#include "SMTPServerSession.src.h"
#include "SMTPServerExceptions.src.h"
#include "SMTPAuthentication.src.h"
#include "../../parsers/mime.src.h"
#include "../../models/LocalDomain.src.h"
#include "../../workers/TransmissionWorker.src.h"

using namespace FSMTP::Parsers;
using namespace FSMTP;
using namespace FSMTP::SMTP;
using namespace FSMTP::Models;
using namespace FSMTP::Networking;
using namespace FSMTP::Workers;
using namespace Sockets;

namespace FSMTP::Server
{
	class SMTPServer
	{
	public:
		SMTPServer() noexcept;
		~SMTPServer() noexcept;

		SMTPServer &listenServer();
		SMTPServer &createContext();
		SMTPServer &startHandler(const bool newThread);

		bool handleCommand(shared_ptr<ClientSocket> client, const ClientCommand &command, SMTPServerSession &session);

		std::vector<SMTPServiceFunction> s_Services;
	private:
		unique_ptr<ServerSocket> s_SSLSocket;
		unique_ptr<ServerSocket> s_PlainSocket;
		unique_ptr<SSLContext> s_SSLContext;
		Logger s_Logger;
	};
}
