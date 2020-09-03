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

#include "SMTPMessageComposer.src.h"

namespace FSMTP::Mailer::Composer
{	
	static char _randomDict[] = {
		'a','b','c','d','e','f','g','h','i','j',
		'k','l','m','n','o','p','q','r','s','t',
		'u','v','w','x','y','z','A','B','C','D',
		'E','F','G','H','I','J','K','L','M','N',
		'O','P','Q','R','S','T','U','V','W','X',
		'Y','Z','1','2','3','4','5','6','7','8',
		'9','0'
	};

	static char _boundaryDict[] = {
		'a','b','c','d','e','f','g','h','i','j',
		'k','l','m','n','o','p','q','r','s','t',
		'u','v','w','x','y','z','A','B','C','D',
		'E','F','G','H','I','J','K','L','M','N',
		'O','P','Q','R','S','T','U','V','W','X',
		'Y','Z','1','2','3','4','5','6','7','8',
		'9','0'
	};

	string generateBoundary(void) {
		string res = "--";
		
		// Prepares the random engine and then generates the random strings
		random_device rd;
		mt19937 re(rd());
		uniform_int_distribution<int> dict(0, sizeof (_boundaryDict) - 1);

		// Generates the MessageID random part
		for (size_t i = 0; i < 38; i++) {
			res += _boundaryDict[dict(re)];
		}

		return res;
	}

	string generateMessageID(void) {
		auto &conf = Global::getConfig();
		string res = "<";

		random_device rd;
		mt19937 re(rd());
		uniform_int_distribution<int> dict(0, sizeof (_randomDict) - 1);

		for (size_t i = 0; i < 64; i++) {
			res += _randomDict[dict(re)];
		}

		res += '@';
		res += conf["smtp"]["client"]["domain"].asCString();
		res += ">";

		return res;
	}

	string compose(MailComposerConfig &config) {
		#ifdef _SMTP_DEBUG
		Logger logger("MailComposer", LoggerLevel::DEBUG);
		Timer t("MessageComposer", logger);
		logger << "Begonnen met opbouw van MIME Message" << ENDL;
		#endif

		auto &conf = Global::getConfig();
		string res;

		// ======================================
		// Prepares the default variables
		//
		// These can be the subject headers
		// - etcetera
		// ======================================

		// Gets the current date and turns it into an string
		char dateBuffer[64];
		time_t rawTime;
		struct tm *timeInfo = nullptr;
		string dateValue;

		time(&rawTime);
		timeInfo = localtime(&rawTime);
		strftime(dateBuffer, sizeof (dateBuffer), "%a, %-d %b %Y %T (%Z)", timeInfo);
		dateValue = dateBuffer;
		DEBUG_ONLY(logger << "Verwerkings datum: " << dateValue << ENDL);

		// Generates the message id
		string messageID = generateMessageID();
		DEBUG_ONLY(logger << "Bericht ID: " << messageID << ENDL);

		// Generates the to and from
		string messageTo = EmailAddress::addressListToString(config.m_To);
		string messageFrom = EmailAddress::addressListToString(config.m_From);
		DEBUG_ONLY(logger << "Van: " << messageFrom << ENDL);
		DEBUG_ONLY(logger << "Naar: " << messageTo << ENDL);

		// Reduces the whitespace
		string subject;
		reduceWhitespace(config.m_Subject, subject);

		// Prepares the default headers
		config.m_Headers.push_back(EmailHeader{"X-Mailer", "FSMTP-V2"});
		config.m_Headers.push_back(EmailHeader{"X-Fannst-NodeID", conf["node_name"].asCString()});
		config.m_Headers.push_back(EmailHeader{"X-Made-By", "Luke A.C.A. Rieff"});
		config.m_Headers.push_back(EmailHeader{"Subject", subject});
		config.m_Headers.push_back(EmailHeader{"Date",  dateValue});
		config.m_Headers.push_back(EmailHeader{"MIME-Version",  "1.0"});
		config.m_Headers.push_back(EmailHeader{"Message-ID", messageID});
		config.m_Headers.push_back(EmailHeader{"To",  messageTo});
		config.m_Headers.push_back(EmailHeader{"From",  messageFrom});

		// ======================================
		// Prepares the message body
		//
		// Just plain text, html or multipart
		// ======================================

		// Checks if default message needs to be specified
		if (config.m_BodySections.size() == 0) {
			DEBUG_ONLY(logger << "sections.size() == 0, generating default one ..." << ENDL);

			// Creates the default message body, since someone did not
			// - specify any body
			config.m_BodySections.push_back(EmailBodySection{
				"<p>\n"
				"<strong>Roses are red.</strong><br />\n"
				"<small>Violets are blue.</small><br />\n"
				"<em>Someone did NOT specify an message body.</em>"
				"</p>",
				EmailContentType::ECT_TEXT_HTML,
				{},
				0,
				EmailTransferEncoding::ETE_QUOTED_PRINTABLE
			});
		}

		if (config.m_BodySections.size() == 1) {
			// Checks the content type, if it is html we just want to turn it into
			// - plain text and make an new section for i
			if (config.m_BodySections[0].e_Type == EmailContentType::ECT_TEXT_HTML)
			{
				DEBUG_ONLY(logger << "HTML Only found, generating text version ..." << ENDL);
				config.m_BodySections.push_back(EmailBodySection{
					generateTextFromHTML(config.m_BodySections[0].e_Content),
					EmailContentType::ECT_TEXT_PLAIN,
					{},
					1,
					EmailTransferEncoding::ETE_QUOTED_PRINTABLE
				});
			}
		}

		// ======================================
		// Generates the MIME message
		//
		// This may be text only, but if more
		// - specified, we will use multipart body
		// ======================================

		// Checks if we just want to generate text only
		if (
			config.m_BodySections.size() == 1 &&
			config.m_BodySections[0].e_Type == EmailContentType::ECT_TEXT_PLAIN
		) {
			DEBUG_ONLY(logger << "Text only detected, generating headers ..." << ENDL);

			// Generates the content type, and appends the body
			config.m_Headers.emplace_back(EmailHeader{
				"Content-Type",
				contentTypeToString(config.m_BodySections[0].e_Type)
			});
			res += generateBodySection(
				config.m_BodySections[0].e_Content,
				config.m_BodySections[0].e_TransferEncoding,
				config.m_Headers
			);
		} else {
			// Generates the boundary and sets the content type
			string boundary = generateBoundary();
			string contentType = contentTypeToString(EmailContentType::ECT_MULTIPART_ALTERNATIVE);
			contentType += "; boundary=\"" + boundary + '\"';

			config.m_Headers.emplace_back(EmailHeader{
				"Content-Type",
				contentType
			});
			
			// Generates the basic headers
			res += generateHeaders(config.m_Headers);
			res += "\r\n";

			// Starts looping over the sections and appends them
			// - the result, with the top boundary
			for (EmailBodySection &section : config.m_BodySections) {
				// Generates the headers, if they're not there yet
				if (section.e_Headers.size() == 0) {
					string contentType = contentTypeToString(section.e_Type);
					contentType += "; charset=\"utf-8\"";
					section.e_Headers.push_back(EmailHeader{
						"Content-Type",
						contentType
					});

					section.e_Headers.push_back(EmailHeader{
						"Content-Transfer-Encoding",
						contentTransferEncodingToString(section.e_TransferEncoding)
					});
				}

				// Adds the boundary and content section
				res += "--" + boundary + "\r\n";
				res += generateBodySection(section.e_Content, 
					section.e_TransferEncoding, section.e_Headers);
				res += "\r\n";
			}

			// Adds the end boundary
			res += "--" + boundary + "--\r\n";

		}

		DEBUG_ONLY(logger << "Result: '\n" << res << '\'' << ENDL);
		return res;
	}

	string generateTextFromHTML(const string &raw) {
		regex htmlRegex("<[^<]*>");
		string res;
		res.reserve(raw.size());

		// Replaces the stuff
		regex_replace(back_inserter(res), raw.begin(), raw.end(), htmlRegex, "");

		return res;
	}

	string generateHeaders(const vector<EmailHeader> &headers) {
		string res;
		for (const EmailHeader &header : headers)
			res += header.e_Key + ": " + header.e_Value + "\r\n";
		return res;
	}

	string encodeMessageBody(
		const string &raw, 
		const EmailTransferEncoding encoding
	) {
		string normalized = normalizeMessageBody(raw);
		string res;

		// Checks which encoding to use
		switch (encoding)
		{
			case EmailTransferEncoding::ETE_QUOTED_PRINTABLE: {
				// Starts encoding the message, and the specific chars
				// - to hex
				res = encodeQuotedPrintable(normalized);
				break;
			}
			default: {
				res = normalized;
			}
		}

		return res;
	}

	string generateBodySection(
		const string &body,
		const EmailTransferEncoding encoding,
		const vector<EmailHeader> &headers
	) {
		string res;

		// Generates the headers
		res += generateHeaders(headers);
		res += "\r\n";

		// Encodes the body
		res += encodeMessageBody(
			body, 
			encoding
		);

		return res;
	}

	string normalizeMessageBody(const string &raw) {
		string res;
		string temp;
		temp.reserve(raw.size());

		// Removes duplicated whitespace
		reduceWhitespace(raw, temp);

		// Loops over the lines, and removes the prefix
		// - and suffix whitespace
		stringstream stream(temp);
		string token;
		bool previousLineWasEmpty = true;
		while (getline(stream, token, '\n')) {
			// Removes the '\r' if there
			if (!token.empty() && token[token.size() - 1] == '\r') token.pop_back();

			// Checks if there are two empty lines, if so
			// - remove the second one
			if (token.empty()) {
				if (previousLineWasEmpty)
					continue;
				previousLineWasEmpty = true;
				res += '\n';
				continue;
			} else previousLineWasEmpty = false;

			// Removes the prefix and suffix whitespace, and pushes
			// - the data to the result
			removeFirstAndLastWhite(token);
			res += token + '\n';
		}

		return res;
	}
}