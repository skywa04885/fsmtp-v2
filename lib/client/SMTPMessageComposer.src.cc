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
		'9','0','_','+','-'
	};

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
		std::uniform_int_distribution<int> dict(0, sizeof (_randomDict));

		// Generates the MessageID random part
		for (std::size_t i = 0; i < 64; i++)
		{
			res += _randomDict[dict(re)];
		}

		// Appends the domain and closing bracket
		res += '@';
		res += _SMTP_SERVICE_DOMAIN;
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
		DEBUG_ONLY(Logger logger("MailComposer", LoggerLevel::DEBUG));
		DEBUG_ONLY(logger << "Begonnen met opbouw van MIME Message" << ENDL);
		std::string res;

		// ======================================
		// Prepares the default variables
		//
		// These can be the subject headers
		// - etcetera
		// ======================================

		// Gets the current date and turns it into an string
		char dateBuffer[128];
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

		// Prepares the default headers
		config.m_Headers.push_back(EmailHeader{"X-Mailer", "FSMTP-V2"});
		config.m_Headers.push_back(EmailHeader{"Subject", config.m_Subject});
		config.m_Headers.push_back(EmailHeader{"Date",  dateValue});
		config.m_Headers.push_back(EmailHeader{"MIME-Version",  "1.0"});
		config.m_Headers.push_back(EmailHeader{"Message-ID",  messageID});
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
				"<em>Someone did NOT specify an message body.</em>\n"
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
					EmailTransferEncoding::ETE_7BIT
				});
			}
		}



		// ======================================
		// Generates the MIME message
		//
		// This may be text only, but if more
		// - specified, we will use multipart body
		// ======================================

		// Loops over the body sections and adds the final CRLF if not there
		for (EmailBodySection &section : config.m_BodySections)
			if (section.e_Content[section.e_Content.size() - 1] != '\n')
				section.e_Content += "\n";

		// Checks if we just want to generate text only
		if (
			config.m_BodySections.size() == 1 &&
			config.m_BodySections[0].e_Type == EmailContentType::ECT_TEXT_PLAIN)
		{
			DEBUG_ONLY(logger << "Text only detected, generating headers ..." << ENDL);

			// Generates the headers and appends them to the result,
			// - nex to that we also add the content type first
			config.m_Headers.emplace_back(EmailHeader{
				"Content-Type",
				contentTypeToString(config.m_BodySections[0].e_Type)
			});
			res += generateHeaders(config.m_Headers);
			res += "\r\n";

			// Encodes the body if required, and appends the body
			res += encodeMessageBody(
				config.m_BodySections[0].e_Content, 
				config.m_BodySections[0].e_TransferEncoding
			);
		}

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
		std::regex_replace(std::back_inserter(res), raw.begin(), raw.end(), htmlRegex, "");
		return res;
	}

	/**
	 * Turns an vector of email headers into valid headers
	 *
	 * @Param {const std::vector<EmailHeader> &} headers
	 * @Return {std::string}
	 */
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
		// Checks which encoding to use
		switch (encoding)
		{
			case EmailTransferEncoding::ETE_QUOTED_PRINTABLE:
			{
				// Starts encoding the message, and the specific chars
				// - to hex
				return encodeQuotedPrintable(raw);
			}
			default: return raw;
		}
	}
}