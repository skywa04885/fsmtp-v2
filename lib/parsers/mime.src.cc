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

#include "mime.src.h"

namespace FSMTP::Parsers::MIME
{
	/**
	 * Parses the headers
	 *
	 * @Param {const std::string &} raw
	 * @Param {std::vector<MimeHeader> &} headers
	 * @Param {const bool &} removeMsGarbage
	 * @Param {void}
	 */
	void parseHeaders(
		const std::string &raw,
		std::vector<EmailHeader> &headers, 
		const bool &removeMsGarbage
	)
	{
		DEBUG_ONLY(Logger logger("HeaderParser", LoggerLevel::PARSER));

		// Starts splitting the headers up
		std::stringstream stream(raw);
		std::string token;
		std::size_t lineIndex = 0;
		while (std::getline(stream, token))
		{
			// Removes the '\r', and checks if
			// - it is empty, if empty just ignore
			if (token[token.size() - 1] == '\r') token.pop_back();
			if (!token.empty())
			{
				// Splits the header, and verifies
				// - if the header is in the correct format
				std::size_t colonPos = token.find_first_of(':');
				if (colonPos == std::string::npos)
				{
					std::string error = "Syntax error on line ";
					error += std::to_string(lineIndex);
					error += " no colon found: '";
					error += token;
					error += '\''; 
					throw std::runtime_error(error);
				}
				std::string key = token.substr(0, colonPos);
				std::string value = token.substr(colonPos+1);
				removeFirstAndLastWhite(value);
				removeFirstAndLastWhite(key);

				// Checks if the header is bullshit, if so
				// - we will simply ignore it
				if (removeMsGarbage && (key[0] == 'X' || key[0] == 'x'))
				{
					DEBUG_ONLY(logger << "Microsoft rotzooi header genegeerd: " << key << ENDL);
					continue;
				}

				// Makes the key lower case
				std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){
					return std::tolower(c);
				});

				// Appends the headers to the final headers
				headers.emplace_back(EmailHeader{key, value});

				// Finishes the current loop and increments
				// - the line index
				DEBUG_ONLY(logger << "Header [no:" << lineIndex << "]: '" << key << "' -> '" << value << "'" << ENDL);
				lineIndex++;
			}
		}
	}

	/**
	 * Parses the headers
	 *
	 * @Param {const std::string &} raw
	 * @Param {FullEmail &} email
	 * @Param {const bool &} removeMsGarbage
	 * @Param {void}
	 */
	void parseMessage(
		std::string raw,
		FullEmail& email,
		const bool &removeMsGarbage
	)
	{
	}

	/**
	 * Splits the headers and body
	 *
	 * @Param {const std::string &} raw
	 * @Param {std::string &} headers
	 * @Param {std::string &} body
	 * @Return {void}
	 */
	void splitHeadersAndBody(
		const std::string &raw,
		std::string &headers,
		std::string &body
	)
	{
		// Starts the splitting process
		std::stringstream stream(raw);
		std::string token;
		bool isBody = false;
		while (std::getline(stream, token))
		{
			// Removes the '\r'
			if (token[token.size() - 1] == '\r') token.pop_back();

			// Checks if the string is empty
			if (!isBody && token.empty())
			{
				isBody = true;
				continue;
			}

			// Appends the strings
			if (isBody)
			{
				body += token;
				body += "\r\n";
			} else
			{
				headers += token;
				headers += "\r\n";
			}
		}
	}

	/**
	 * Joins separated lines inside of the messae
	 * these are some or another way required ;(
	 *
	 * @Parma {std::string &} raw
	 */
	void joinMessageLines(
		std::string &raw
	)
	{
		std::string res;

		// Starts the splitting process
		std::stringstream stream(raw);
		std::string token;
		std::size_t i = 0;
		while (std::getline(stream, token))
		{
			// Removes the '\r'
			if (token[token.size() - 1] == '\r') token.pop_back();

			if (token[0] == ' ' || token[0] == '\t')
			{
				res += token;
			} else {
				if (i != 0) res += "\r\n";
				res += token;
			}

			i++;
		}

		// Overwrites the source
		raw = res;
	}

	/**
	 * Parses multipart email body
	 *
	 * @Param {const std::string &} raw
	 * @Param {EmailContentType &} contentType
	 * @Param {std::vector<EmailBodySection> &} sections
	 * @Return {void}
	 */
	void parseMultipartBody(
		const std::string &raw,
		const EmailContentType &contentType,
		std::vector<EmailBodySection> &sections,
		const std::string &boundary
	)
	{
		// Checks which content type it is,
		// - and the processes the email accordingly
		switch (contentType)
		{
			case EmailContentType::ECT_MULTIPART_ALTERNATIVE: case EmailContentType::ECT_MULTIPART_MIXED:
			{
				// ======================================================
				// Start multipart/alternative parser
				// ======================================================

				// Starts 
				std::stringstream stream(raw);
				std::string token, sectionTemp;
				std::size_t contentIndex = 0;
				bool sectionStarted = false;
				while (std::getline(stream, token))
				{
					// Removes the '\r'
					if (token[token.size() - 1] == '\r') token.pop_back();	

					// Checks if an section end or start possible occured
					if (token.substr(0, 2) == "--")
					{
						// Checks if it is the end boundary and if not
						// - we will still check if it is an regular boundary
						bool end = false;
						if (token.substr(2, token.size() - 4) == boundary) end = true;

						if (token.substr(2) == boundary || end)
						{
							// Checks if it is the first round
							// - if so we just want to continue nothing to append
							// - but if it is the end already, we will throw exception
							// - since there was no content
							if (contentIndex == 0 && !end)
							{
								sectionStarted = true;
								contentIndex++;
								continue;
							} else if (contentIndex == 0 && end) throw std::runtime_error("Empty body is not allowed");

							EmailBodySection section;
							section.e_Index = contentIndex;
							parseBodySection(sectionTemp, section, false);
							sections.push_back(section);

							// Clears the temp, and finishes the round
							// - by incrementing the index
							contentIndex++;
							sectionTemp.clear();
							
							if (end) break;
							else continue;
						}
					}

					// Checks if an section has started
					if (sectionStarted)
					{
						sectionTemp += token;
						sectionTemp += "\r\n";
					}
				}

				// ======================================================
				// End multipart/alternative parser
				// ======================================================
				break;
			}
			default: throw std::runtime_error("Multipart body type not implemented !");
		}
	}

	/**
	 * Parses header sub arguments
	 *
	 * @Param {const std::string &} raw
	 * @Return {void}
	 */
	std::vector<std::string> parseHeaderSubtext(const std::string &raw)
	{
		// Starts the looping process
		std::vector<std::string> ret;
		std::stringstream stream(raw);
		std::string token;
		while (std::getline(stream, token, ';'))
		{
			ret.push_back(token);
		}
		return ret;
	}

	void parseBodySection(
		const std::string &raw,
		EmailBodySection &section,
		bool plain
	)
	{
		std::string body;
	
		if (!plain)
		{
			std::string headers;

			// Sets the default content types
			section.e_Type = EmailContentType::ECT_NOT_FOUND;
			section.e_TransferEncoding = EmailTransferEncoding::ETE_NOT_FOUND;

			// Parses the section temp into valid content
			// - first starting with the headers
			splitHeadersAndBody(raw, headers, body);

			// Parses the headers and stores
			// - them inside of the vector
			parseHeaders(headers, section.e_Headers, false);

			// Gets some of the header fields we 
			// - require, such as the content type
			for (const EmailHeader &h : section.e_Headers)
			{
				if (h.e_Key == "content-type")
				{
					// Parses the sub values, and checks if the
					// - subtext is valid
					std::vector<std::string> vals = parseHeaderSubtext(h.e_Value);
					if (vals.size() == 0)
						throw std::runtime_error("Content type not found !");

					section.e_Type = stringToEmailContentType(vals[0]);
				} else if (h.e_Key == "content-transfer-encoding")
				{
					// Parses the transfer encoding
					section.e_TransferEncoding = stringToEmailTransferEncoding(h.e_Value);
				}
			}

			// Checks if the data is there, if not throw error
			if (section.e_Type == EmailContentType::ECT_NOT_FOUND)
				throw std::runtime_error("Content type not found");
			if (section.e_TransferEncoding == EmailTransferEncoding::ETE_NOT_FOUND)
				throw std::runtime_error("Content transfer encoding not found");

			if (section.e_Type == EmailContentType::ECT_NOT_FUCKING_KNOWN)
				throw std::runtime_error("Content type not implemented");
			if (section.e_TransferEncoding == EmailTransferEncoding::ETE_NOT_FUCKING_KNOWN)
				throw std::runtime_error("Content transfer encoding not implemented");
		} else body = raw;

		// Starts decoding the body section if required
		// - for example "7bit" or "quoted-printable"
		switch (section.e_TransferEncoding)
		{
			case EmailTransferEncoding::ETE_QUOTED_PRINTABLE:
			{
				section.e_Content = decodeQuotedPrintable(body);
				break;
			}
			default: section.e_Content = body;								
		}
	}

	// ================================
	// New parser ( better )
	// ================================

	void parseRecursive(
		std::string raw,
		FullEmail& email,
		std::size_t i
	)
	{
		Logger logger(
			"RecursiveParserRound:" + std::to_string(i),
			LoggerLevel::PARSER
		);
		logger << "Nieuwe ronde gestart ..." << ENDL;

		std::string body;
		std::string headers;
		std::vector<EmailHeader> parsedHeaders;
		EmailContentType parsedContentType = EmailContentType::ECT_NOT_FOUND;
		EmailTransferEncoding parsedTransferEncoding = EmailTransferEncoding::ETE_NOT_FOUND;

		// Splits the headers and body, so we can parse
		// - them separate
		logger << "Begonnen met splitsen van headers en body" << ENDL;
		splitHeadersAndBody(raw, headers, body);

		// Parses the headers, and then processes the body
		// - first the headers so we can check how to treat the body
		parseHeaders(headers, parsedHeaders, true);

		// Gets the required parameters from the headers, so
		// - we can treat the body, and store the data correctly
		for (const EmailHeader &header : parsedHeaders)
		{
			if (header.e_Key == "content-type")
			{
				// Parses the arguments from the header subtext, and then
				// - gets the content type, and if there the content-transfer-encoding
				std::vector<string> subtext = parseHeaderSubtext(header.e_Value)

				if (subtext.size() > 0) parsedContentType = stringToEmailContentType(subtext[0]);
				else throw std::runtime_error("Empty content-type header is not allowed !");

				if (subtext.size() >= 2)
			} else if (header.e_Key == "content-transfer-encoding")
			{
				parsedContentType = stringToEmailTransferEncoding(header.e_Value);
			}
		}

		// Checks if this is the first recursive round
		// - which would mean we need to modify the email
		// - instead of creating an new 
		if (i == 0)
		{
			logger << "Eerste ronde gedetecteerd, opslaan in FullEmail & ..." << ENDL;
		} else
		{
			logger << "Sub ronde gedetecteerd, invoeging van sectie in vector ..." << ENDL;
		}
	}
}