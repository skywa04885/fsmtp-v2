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

#include "SMTPMessageComposer.src.h"
#include "SMTPClientSession.src.h"

#include "../../default.h"
#include "../../general/Global.src.h"
#include "../../networking/sockets/ClientSocket.src.h"
#include "../../models/Email.src.h"
#include "../../general/Logger.src.h"
#include "../../dns/Resolver.src.h"
#include "../../dkim/DKIMSigner.src.h"
#include "../Response.src.h"
#include "../Command.src.h"
#include "../../networking/IP.src.h"

using namespace FSMTP::Sockets;
using namespace FSMTP::Models;
using namespace FSMTP::SMTP;
using namespace FSMTP::Mailer::Composer;

namespace FSMTP::Mailer::Client {
	enum SMTPClientActionType {
		SAT_HELO,
		SAT_START_TLS,
		SAT_EHLO,
		SAT_MAIL_FROM,
		SAT_RCPT_TO,
		SAT_DATA
	};

	struct SMTPClientServer {
		string address;
		Networking::IP::Protocol protocol;
	};

	void __smtpClientServerLog(Logger &logger, const SMTPClientServer &server);
	
	template<typename T>
	void __smtpClientsServerLog(Logger &logger, const T &server) {
		for_each(server.begin(), server.end(), [&](const SMTPClientServer &server) {
			__smtpClientServerLog(logger, server);
		});
	}

	struct SMTPClientTarget {
		vector<SMTPClientServer> servers;
		EmailAddress address;
	};

	class SMTPClient {
	public:
		explicit SMTPClient(bool s_Silent);

		SMTPClient &prepare(
			const vector<EmailAddress> to,
			const vector<EmailAddress> from,
			const string &message
		);
		vector<SMTPClientServer> getServers(const string &domain);
		SMTPClient &prepare(MailComposerConfig &config);
		SMTPClient &beSocial(void);
		SMTPClient &configureRecipients(const vector<EmailAddress> &addresses);
		SMTPClient &addError(const string &address, const string &message);
		SMTPClient &printReceived(const int32_t code, const string &args);
		SMTPClient &printSent(const string &mess);
		SMTPClient &reset();
		SMTPClient &sign(const string &message);

		bool s_Silent;
		Logger s_Logger;
		string s_TransportMessage;
		vector<SMTPClientTarget> s_Targets;
		json s_ErrorLog;
		EmailAddress s_MailFrom;
		size_t s_ErrorCount;
	};
}