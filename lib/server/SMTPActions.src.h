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
#include <stdexcept>
#include <variant>
#include <memory>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../smtp/Command.src.h"
#include "../smtp/Response.src.h"
#include "../networking/SMTPSocket.src.h"
#include "SMTPServerExceptions.src.h"
#include "../models/email.src.h"
#include "../general/logger.src.h"
#include "../general/macros.src.h"
#include "../general/connections.src.h"
#include "../models/LocalDomain.src.h"
#include "../models/Account.src.h"
#include "SMTPServerSession.src.h"

using namespace FSMTP::SMTP;
using namespace FSMTP::Networking;
using namespace FSMTP::Models;
using namespace FSMTP::Connections;

namespace FSMTP::Server::Actions
{
	typedef struct
	{
		const ClientCommand &command;
		struct sockaddr_in *sAddr;
		const bool &esmtp;
		int32_t &fd;
		SSL *ssl;
	} BasicActionData;

	/**
	 * Handles the "HELO" / "EHLO" command of the SMTP Server
	 *
	 * @Param {BasicActionData &data} data
	 * @Return {void}
	 */
	void actionHelloInitial(
		BasicActionData &data
	);

	/**
	 * Handles the "MAIL FROM" command of the SMTP Server
	 *
	 * @Param {BasicActionData &} data
	 * @Param {Logger &} logger
	 * @Param {std::unique_ptr<CassandraConnection> &} database
	 * @Param {SMTPServerSession &} session
	 * @Return {void}
	 */
	void actionMailFrom(
		BasicActionData &data,
		Logger& logger,
		std::unique_ptr<CassandraConnection> &database,
		SMTPServerSession &session
	);

	/**
	 * Handles the "RCPT TO" command of the SMTP Server
	 *
	 * @Param {BasicActionData &} data
	 * @Param {Logger&} logger
	 * @Param {std::unique_ptr<CassandraConnection> &} database
	 * @Param {SMTPServerSession &} session
	 * @Return {void}
	 */
	void actionRcptTo(
		BasicActionData &data,
		Logger& logger,
		std::unique_ptr<CassandraConnection> &database,
		SMTPServerSession &session
	);
}
