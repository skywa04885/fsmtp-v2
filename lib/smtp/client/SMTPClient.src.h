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
#include "../../general/Global.src.h"

#include "../../networking/sockets/ClientSocket.src.h"
#include "../../models/Email.src.h"
#include "../../general/Logger.src.h"
#include "../../networking/DNS.src.h"
#include "../Response.src.h"
#include "../Command.src.h"
#include "../../dkim/DKIM.src.h"
#include "SMTPMessageComposer.src.h"
#include "SMTPClientSession.src.h"

using namespace FSMTP::Sockets;
using namespace FSMTP::Models;
using namespace FSMTP::SMTP;
using namespace FSMTP::Mailer::Composer;

namespace FSMTP::Mailer::Client {
	typedef enum : uint32_t {
		SAT_HELO,
		SAT_START_TLS,
		SAT_EHLO,
		SAT_MAIL_FROM,
		SAT_RCPT_TO,
		SAT_DATA
	} SMTPClientActionType;

	typedef struct {
		string s_Address;
		string s_Message;
	} SMTPClientError;

	typedef struct {
		vector<string> t_Servers;
		EmailAddress t_Address;
	} SMTPClientTarget;

	class SMTPClient {
	public:
		explicit SMTPClient(bool s_Silent);

		void prepare(
			const vector<EmailAddress> to,
			const vector<EmailAddress> from,
			const string &message
		);

		void prepare(MailComposerConfig &config);
		void beSocial(void);
		void configureRecipients(const vector<EmailAddress> &addresses);
		void addError(const string &address, const string &message);
		void printReceived(const int32_t code, const string &args);
		void printSent(const string &mess);

		bool s_Silent;
		Logger s_Logger;
		string s_TransportMessage;
		vector<SMTPClientTarget> s_Targets;
		vector<SMTPClientError> s_ErrorLog;
		EmailAddress s_MailFrom;
		size_t s_ErrorCount;
	};
}