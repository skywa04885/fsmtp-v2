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

extern Json::Value _config;

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

	/**
	 * Generates an random boundary
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string generateBoundary(void)
	{
		std::string res = "--";
		
		// Prepares the random engine and then generates the random strings
		std::random_device rd;
		std::mt19937 re(rd());
		std::uniform_int_distribution<int> dict(0, sizeof (_boundaryDict) - 1);

		// Generates the MessageID random part
		for (std::size_t i = 0; i < 38; i++)
		{
			res += _boundaryDict[dict(re)];
		}

		return res;
	}

	/**
	 * Generates an MessageID
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string generateMessageID(void)
	{
		std::string res = "<";

		// Prepares the random engine and then generates the random strings
		// - which will be appended with the domain
		std::random_device rd;
		std::mt19937 re(rd());
		std::uniform_int_distribution<int> dict(0, sizeof (_randomDict) - 1);

		// Generates the MessageID random part
		for (std::size_t i = 0; i < 64; i++)
		{
			res += _randomDict[dict(re)];
		}

		// Appends the domain and closing bracket
		res += '@';
		res += _config["smtp"]["client"]["domain"].asCString();
		res += ">";

		return res;
	}

	/**
	 * Composes an MIME message using specified configuration
	 *
	 * @Param {const MailComposerConfig &} config
	 * @Return {std::string}
	 */
	std::string compose(MailComposerConfig &config)
	{
		#ifdef _SMTP_DEBUG
		Logger logger("MailComposer", LoggerLevel::DEBUG);
		Timer t("MessageComposer", logger);
		logger << "Begonnen met opbouw van MIME Message" << ENDL;
		#endif

		std::string res;

		// ======================================
		// Prepares the default variables
		//
		// These can be the subject headers
		// - etcetera
		// ======================================

		// Gets the current date and turns it into an string
		char dateBuffer[64];
		std::time_t rawTime;
		struct tm *timeInfo = nullptr;
		std::string dateValue;

		time(&rawTime);
		timeInfo = localtime(&rawTime);
		strftime(dateBuffer, sizeof (dateBuffer), "%a, %d %b %Y %T %Z", timeInfo);
		dateValue = dateBuffer;
		DEBUG_ONLY(logger << "Verwerkings datum: " << dateValue << ENDL);

		// Generates the message id
		std::string messageID = generateMessageID();
		DEBUG_ONLY(logger << "Bericht ID: " << messageID << ENDL);

		// Generates the to and from
		std::string messageTo = EmailAddress::addressListToString(config.m_To);
		std::string messageFrom = EmailAddress::addressListToString(config.m_From);
		DEBUG_ONLY(logger << "Van: " << messageFrom << ENDL);
		DEBUG_ONLY(logger << "Naar: " << messageTo << ENDL);

		// Reduces the whitespace
		std::string subject;
		reduceWhitespace(config.m_Subject, subject);

		// Prepares the default headers
		config.m_Headers.push_back(EmailHeader{"X-Mailer", "FSMTP-V2"});
		config.m_Headers.push_back(EmailHeader{"X-Fannst-NodeID", _config["node_name"].asCString()});
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
		if (config.m_BodySections.size() == 0)
		{
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

		if (config.m_BodySections.size() == 1)
		{
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
			config.m_BodySections[0].e_Type == EmailContentType::ECT_TEXT_PLAIN)
		{
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
		} else
		{
			// Generates the boundary and sets the content type
			std::string boundary = generateBoundary();
			std::string contentType = contentTypeToString(EmailContentType::ECT_MULTIPART_ALTERNATIVE);
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
			for (EmailBodySection &section : config.m_BodySections)
			{
				// Generates the headers, if they're not there yet
				if (section.e_Headers.size() == 0)
				{
					std::string contentType = contentTypeToString(section.e_Type);
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

	/**
	 * Turns an HTML message into an text message
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string generateTextFromHTML(const std::string &raw)
	{
		std::regex htmlRegex("<[^<]*>");
		std::string res;
		res.reserve(raw.size());

		// Replaces the stuff
		std::regex_replace(std::back_inserter(res), raw.begin(), raw.end(), htmlRegex, "");

		return res;
	}

	/**
	 * Turns an vector of email headers into valid headers
	 *
	 * @Param {const std::vector<EmailHeader> &} headers
	 * @Return {std::string}
	 */
	// TODO: Add header length limit and then newline shit
	std::string generateHeaders(const std::vector<EmailHeader> &headers)
	{
		std::string res;
		for (const EmailHeader &header : headers)
			res += header.e_Key + ": " + header.e_Value + "\r\n";
		return res;
	}

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
	)
	{
		std::string normalized = normalizeMessageBody(raw);
		std::string res;

		// Checks which encoding to use
		switch (encoding)
		{
			case EmailTransferEncoding::ETE_QUOTED_PRINTABLE:
			{
				// Starts encoding the message, and the specific chars
				// - to hex
				res = encodeQuotedPrintable(normalized);
				break;
			}
			default:
			{
				res = normalized;
			}
		}

		// Replaces the '\n' with '\r\n'

		return res;
	}

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
	)
	{
		std::string res;

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


	/**
	 * Normalizes an message body, feature set:
	 * 1. Remove duplicated whitespace
	 * 2. Remove whitespace at the start and end of lines
	 *
	 * @Param {const std::string &} raw
	 * @return {std::string}
	 */
	std::string normalizeMessageBody(const std::string &raw)
	{
		std::string res;
		std::string temp;
		temp.reserve(raw.size());

		// Removes duplicated whitespace
		reduceWhitespace(raw, temp);

		// Loops over the lines, and removes the prefix
		// - and suffix whitespace
		std::stringstream stream(temp);
		std::string token;
		bool previousLineWasEmpty = true;
		while (std::getline(stream, token, '\n'))
		{
			// Removes the '\r' if there
			if (!token.empty() && token[token.size() - 1] == '\r') token.pop_back();

			// Checks if there are two empty lines, if so
			// - remove the second one
			if (token.empty())
			{
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