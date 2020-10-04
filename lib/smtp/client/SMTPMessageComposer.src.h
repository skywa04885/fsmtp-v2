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
#include "../../general/hex.src.h"
#include "../../general/cleanup.src.h"
#include "../../general/Timer.src.h"
#include "../../mime/mimev2.src.h"

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
		vector<EmailAddress> m_From = {};
		vector<EmailAddress> m_To = {};
		vector<EmailBodySection> m_BodySections = {};
		vector<MIME::MIMEHeader> m_Headers = {};
		string m_Subject;
	} MailComposerConfig;

	string compose(MailComposerConfig &config);
	string generateMessageID(void);
	string generateTextFromHTML(const string &raw);
	string generateHeaders(const vector<MIME::MIMEHeader> &headers);
	string encodeMessageBody(
		const string &raw, 
		const EmailTransferEncoding encoding
	);
	string generateBodySection(
		const string &body,
		const EmailTransferEncoding encoding,
		const vector<MIME::MIMEHeader> &headers
	);
	string generateBoundary(void);
	string normalizeMessageBody(const string &raw);
}