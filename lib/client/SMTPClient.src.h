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

#include "../networking/SMTPSocket.src.h"
#include "../models/email.src.h"
#include "../general/logger.src.h"
#include "SMTPMessageComposer.src.h"

using namespace FSMTP::Networking;
using namespace FSMTP::Models;
using namespace FSMTP::Mailer::Composer;

namespace FSMTP::Mailer::Client
{
	typedef enum : uint8_t
	{
		SCP_RESOLVE = 0,
		SCP_TRANSMISION_START,
		SCP_TRANSMISION_DATA,
		SCP_TRANSMISION_OTHER
	} SMTPClientPhase;

	typedef struct
	{
		SMTPClientPhase s_Phase;
		std::size_t s_Timestamp;
		std::string s_Message;
	} SMTPClientError;

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
	private:
		bool s_Silent;
		Logger s_Logger;
		std::string s_TransportMessage;
		std::vector<EmailAddress> s_Targets;
		std::vector<SMTPClientError> s_ErrorLog;
	};
}