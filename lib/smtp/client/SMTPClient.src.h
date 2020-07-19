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
#include <iostream>
#include <chrono>
#include <functional>

#include "../../networking/SMTPSocket.src.h"
#include "../../models/Email.src.h"
#include "../../general/Logger.src.h"
#include "../../networking/DNS.src.h"
#include "../Response.src.h"
#include "../Command.src.h"
#include "../../dkim/DKIM.src.h"
#include "SMTPMessageComposer.src.h"
#include "SMTPClientSession.src.h"

using namespace FSMTP::Networking;
using namespace FSMTP::Models;
using namespace FSMTP::SMTP;
using namespace FSMTP::Mailer::Composer;

namespace FSMTP::Mailer::Client
{
	typedef enum : uint32_t
	{
		SAT_HELO,
		SAT_START_TLS,
		SAT_EHLO,
		SAT_MAIL_FROM,
		SAT_RCPT_TO,
		SAT_DATA
	} SMTPClientActionType;

	typedef struct
	{
		std::string s_Address;
		std::string s_Message;
	} SMTPClientError;

	typedef struct
	{
		std::vector<std::string> t_Servers;
		EmailAddress t_Address;
	} SMTPClientTarget;

	class SMTPClient
	{
	public:
		/**
		 * Default constructor for the SMTPClient
		 *
		 * @Param {bool} s_Silent
		 * @Return {void}
		 */
		explicit SMTPClient(bool s_Silent);

		void prepare(
			const std::vector<EmailAddress> to,
			const std::vector<EmailAddress> from,
			const std::string &message
		);

		/**
		 * Composes the email message, and sets some options
		 * - inside of the class, such as the message and targets
		 * 
		 * @Param {MailComposerConfig &} config
		 * @Return {void}
		 */
		void prepare(MailComposerConfig &config);

		/**
		 * ( May take a while )
		 * Orders the computer to talk with the other
		 * - servers and transmit the message
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void beSocial(void);

		/**
		 * Resolves the recipients servers
		 *
		 * @Param {const std::vector<EmailAddress> &} addresses
		 * @Return {void}
		 */
		void configureRecipients(const std::vector<EmailAddress> &addresses);

		/**
		 * Adds an error to the error log
		 *
		 * @Param {const std::string &} address
		 * @Param {const std::string &} message
		 * @Return {void}
		 */
		void addError(
			const std::string &address,
			const std::string &message
		);

		/**
		 * Prints something we recieved from the client
		 *
		 * @Param {const int32_t} code
		 * @Param {const std::string &} args\
		 * @Return {void}
		 */
		void printReceived(const int32_t code, const std::string &args);

		/**
		 * Prints something we sent to the console
		 *
		 * @Param {const std::string &} mess
		 * @Return {void}
		 */
		void printSent(const std::string &mess);

		bool s_Silent;
		Logger s_Logger;
		std::string s_TransportMessage;
		std::vector<SMTPClientTarget> s_Targets;
		std::vector<SMTPClientError> s_ErrorLog;
		EmailAddress s_MailFrom;
		std::size_t s_ErrorCount;
	};
}