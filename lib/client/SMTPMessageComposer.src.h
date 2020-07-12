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

#include <vector>
#include <ctime>
#include <cstdint>
#include <regex>
#include <string>
#include <random>

#include "../models/email.src.h"
#include "../general/encoding.src.h"
#include "../general/macros.src.h"
#include "../general/hex.src.h"
#include "../general/cleanup.src.h"
#include "../general/Timer.src.h"

using namespace FSMTP::Models;
using namespace FSMTP::Cleanup;
using namespace FSMTP::Encoding;

namespace FSMTP::Mailer::Composer
{
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

	/**
	 * Composes an MIME message using specified configuration
	 *
	 * @Param {const MailComposerConfig &} config
	 * @Return {std::string}
	 */
	std::string compose(MailComposerConfig &config);

	/**
	 * Generates an MessageID
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string generateMessageID(void);

	/**
	 * Turns an HTML message into an text message
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string generateTextFromHTML(const std::string &raw);

	/**
	 * Turns an vector of email headers into valid headers
	 *
	 * @Param {const std::vector<EmailHeader> &} headers
	 * @Return {std::string}
	 */
	std::string generateHeaders(const std::vector<EmailHeader> &headers);

	/**
	 * Encodes an message body
	 *
	 * @Param {const std::string &} raw
	 * @Param {const EmailTransferEncoding} encoding
	 * @Return {std::string}
	 */
	std::string encodeMessageBody(
		const std::string &raw, 
		const EmailTransferEncoding encoding
	);

	/**
	 * Generates an body section
	 *
	 * @Param {const std::string &} body
	 * @Param {const EmailTransferEncoding} encoding
	 * @Param {const std::vector<EmailHeader> &} headers
	 * @Return {std::string}
	 */
	std::string generateBodySection(
		const std::string &body,
		const EmailTransferEncoding encoding,
		const std::vector<EmailHeader> &headers
	);

	/**
	 * Generates an random boundary
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string generateBoundary(void);
}