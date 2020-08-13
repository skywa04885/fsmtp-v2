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
#include "../../models/Email.src.h"
#include "../../general/encoding.src.h"
#include "../../general/macros.src.h"
#include "../../general/hex.src.h"
#include "../../general/cleanup.src.h"
#include "../../general/Timer.src.h"

using namespace FSMTP::Models;
using namespace FSMTP::Cleanup;
using namespace FSMTP::Encoding;

namespace FSMTP::Mailer::Composer {
	/*
		The configuration for the mail composer, this will be used to compose an
		- MIME message we will later transmit using the client
	*/
	typedef struct
	{
		std::vector<EmailAddress> m_From = {};
		std::vector<EmailAddress> m_To = {};
		std::vector<EmailBodySection> m_BodySections = {};
		std::vector<EmailHeader> m_Headers = {};
		std::string m_Subject;
	} MailComposerConfig;

	std::string compose(MailComposerConfig &config);
	std::string generateMessageID(void);
	std::string generateTextFromHTML(const std::string &raw);
	std::string generateHeaders(const std::vector<EmailHeader> &headers);
	std::string encodeMessageBody(
		const std::string &raw, 
		const EmailTransferEncoding encoding
	);
	std::string generateBodySection(
		const std::string &body,
		const EmailTransferEncoding encoding,
		const std::vector<EmailHeader> &headers
	);
	std::string generateBoundary(void);
	std::string normalizeMessageBody(const std::string &raw);
}