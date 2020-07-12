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

#include "../networking/SMTPSocket.src.h"
#include "../models/email.src.h"
#include "../general/logger.src.h"
#include "../networking/DNS.src.h"
#include "SMTPMessageComposer.src.h"
#include "../smtp/Response.src.h"
#include "../smtp/Command.src.h"
#include "SMTPClientSession.src.h"

using namespace FSMTP::Networking;
using namespace FSMTP::Models;
using namespace FSMTP::SMTP;
using namespace FSMTP::Mailer::Composer;

namespace FSMTP::Mailer::Client
{
	typedef enum : uint8_t
	{
		SCP_RESOLVE = 0,
		SCP_CONNECT,
		SCP_MAIL_FROM,
		SCP_RCPT_TO,
		SCP_START_TLS,
		SCP_DATA,
		SCP_OTHER
	} SMTPClientPhase;

	typedef struct
	{
		SMTPClientPhase s_Phase;
		int64_t s_Timestamp;
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

		/**
		 * Composes the email message, and sets some options
		 * - inside of the class, such as the message and targets
		 * 
		 * @Param {MailComposerConfig &config}
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
		 * Adds an error to the error log
		 *
		 * @Param {const SMTPClientPhase} phase
		 * @Param {const std::string &} message
		 * @Return {void}
		 */
		void addError(
			const SMTPClientPhase phase,
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
	private:
		bool s_Silent;
		Logger s_Logger;
		std::string s_TransportMessage;
		std::vector<SMTPClientTarget> s_Targets;
		std::vector<SMTPClientError> s_ErrorLog;
		EmailAddress s_MailFrom;
	};
}