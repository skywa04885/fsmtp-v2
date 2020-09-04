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
	 * @Param {std::string} raw
	 * @Param {std::vector<MimeHeader> &} headers
	 * @Param {const bool &} removeMsGarbage
	 * @Param {void}
	 */
	void parseHeaders(
		std::string raw,
		std::vector<EmailHeader> &headers, 
		const bool &removeMsGarbage
	)
	{
		MIME::joinMessageLines(raw);
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
				if (colonPos == std::string::npos) continue;
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
	) {
		string res;

		// Splits the document into lines
		stringstream stream(raw);
		string line;
		vector<string> lines = {};
		while (getline(stream, line, '\n')) {
			// Since we can only use one char for the getline, we need
			//  to remove the \r ourselves, after which we push it to
			//  the lines vector
			if (line[line.size() - 1] == '\r') line.pop_back();
			lines.push_back(line);
		}

		// Starts joining the lines
		for (size_t i = 0; i < lines.size(); i++) {
			lines[i].erase(lines[i].find_last_not_of(' ') + 1);
			lines[i].erase(0, lines[i].find_first_not_of(' '));
			if (i + 1 < lines.size() && lines[i+1][0] == ' ') {
				if (res[res.size() - 1] == ';') res += ' ';
				res += lines[i];
			} else {
				res += lines[i];
				res += "\r\n";
			}
		}

		raw = res;
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

	/**
	 * Parses the value of an argument like a="asd"
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string parseSubtextValue(const std::string &raw)
	{
		std::size_t index = raw.find_first_of('=');
		if (index == std::string::npos)
			throw std::runtime_error(EXCEPT_DEBUG("Subtext value could not be splitted"));
		std::string temp = raw.substr(++index);
		return temp.substr(1, temp.size() - 2);
	}

	/**
	 * Parses an mime message in the recursive manner
	 *
	 * @Param {std::string} raw
	 * @Param {FullEmail &} email
	 * @Param {std::size_t} i
	 */
	void parseRecursive(
		std::string raw,
		FullEmail& email,
		std::size_t i
	)
	{
		DEBUG_ONLY(Logger logger(
			"RecursiveParserRound:" + std::to_string(i),
			LoggerLevel::PARSER
		));
		DEBUG_ONLY(logger << "Nieuwe ronde gestart ..." << ENDL);

		std::string body;
		std::string headers;
		std::string boundary;
		std::string finalContent;
		std::vector<EmailHeader> parsedHeaders;
		EmailContentType parsedContentType = EmailContentType::ECT_NOT_FOUND;
		EmailTransferEncoding parsedTransferEncoding = EmailTransferEncoding::ETE_NOT_FOUND;

		// Splits the headers and body, so we can parse
		// - them separate
		DEBUG_ONLY(logger << "Begonnen met splitsen van headers en body" << ENDL);
		splitHeadersAndBody(raw, headers, body);

		// Parses the headers, and then processes the body
		// - first the headers so we can check how to treat the body
		parseHeaders(headers, parsedHeaders, false);

		// Gets the required parameters from the headers, so
		// - we can treat the body, and store the data correctly
		for (const EmailHeader &header : parsedHeaders)
		{
			if (header.e_Key == "content-type")
			{
				// Parses the arguments from the header subtext, and then
				// - gets the content type, and if there the content-transfer-encoding
				std::vector<std::string> subtext = parseHeaderSubtext(header.e_Value);

				if (subtext.size() > 0) parsedContentType = stringToEmailContentType(subtext[0]);
				else throw std::runtime_error(EXCEPT_DEBUG("Empty content-type header is not allowed !"));

				// Checks the content type, and then parses
				// - the correct parameter
				switch (parsedContentType)
				{
					case EmailContentType::ECT_TEXT_PLAIN:
					case EmailContentType::ECT_TEXT_HTML:
					{
						if (subtext.size() >= 2)
							parsedTransferEncoding = stringToEmailTransferEncoding(
								parseSubtextValue(subtext[1]));
						else
							parsedTransferEncoding = EmailTransferEncoding::ETE_7BIT;
						break;
					}
					case EmailContentType::ECT_MULTIPART_ALTERNATIVE:
					case EmailContentType::ECT_MULTIPART_MIXED:
					{
						if (subtext.size() >= 2) boundary = parseSubtextValue(subtext[1]);
						else throw std::runtime_error(EXCEPT_DEBUG("Could not find boundary"));
						break;
					}
				}
			} else if (header.e_Key == "content-transfer-encoding")
			{
				parsedTransferEncoding = stringToEmailTransferEncoding(header.e_Value);
			}
		}

		// Checks if this is the first recursive round
		// - so we can store the basic headers
		if (i == 0)
		{
			DEBUG_ONLY(logger << "Eerste ronde gedetecteerd, opslaan in FullEmail & ..." << ENDL);

			// Gets the required parameters from the headers,
			// - such as the subject and date
			for (const EmailHeader &header : parsedHeaders)
			{
				if (header.e_Key == "subject")
				{
					email.e_Subject = header.e_Value;
				}
				else if (header.e_Key == "date")
				{
					tm timeP;
					strptime(header.e_Value.c_str(), "%a, %d %b %Y %T %Z", &timeP);
					time_t sinceEpoch = timegm(&timeP);
					email.e_Date = static_cast<std::size_t>(sinceEpoch);
				} else if (header.e_Key == "message-id")
				{
					email.e_MessageID = header.e_Value;
				} else if (header.e_Key == "from")
				{
					email.e_From = EmailAddress::parseAddressList(header.e_Value);
				} else if (header.e_Key == "to")
				{
					email.e_To = EmailAddress::parseAddressList(header.e_Value);
				}
			}

			// Sets the headers, since we're in the first round
			//- It's not meant as section
			email.e_Headers = parsedHeaders;
		}

		// Checks how we should process the data further, and if
		// - any more recursive actions are required
		switch (parsedContentType)
		{
			case EmailContentType::ECT_TEXT_PLAIN:
			case EmailContentType::ECT_TEXT_HTML:
			{
				// Checks the encoding, so we can decode
				// - it if required
				switch (parsedTransferEncoding)
				{
					case EmailTransferEncoding::ETE_QUOTED_PRINTABLE:
					{
						finalContent = decodeQuotedPrintable(body);
						break;
					}
					default: finalContent = body;
				}

				break;
			}
			case EmailContentType::ECT_MULTIPART_MIXED:
			case EmailContentType::ECT_MULTIPART_ALTERNATIVE:
			{
				std::vector<std::string> sections = {};
				std::stringstream stream(body);
				std::string token;
				std::string temp;

				DEBUG_ONLY(logger << "Begonnen met opdelen body in secties, met behulp van boundary: " << boundary << ENDL);

				// Loops over the lines, and separates these ones
				// - into separate mime sections
				while (std::getline(stream, token, '\n'))
				{
					if (token[token.size() - 1] == '\r') token.pop_back();

					if (token[0] == '-' && token[1] == '-')
					{
						bool endBoundary = false;

						// Checks if it is the end boundary,
						// - and if it is the case, set bool
						if (token == "--" + boundary + "--") endBoundary = true;

						// Checks if it is the end, or an section start
						// - boundary has been added
						if (endBoundary || token == "--" + boundary)
						{
							if (temp.size() > 0)
								DEBUG_ONLY(logger << "Subsection detected of length: " << temp.size() << ENDL);

							// Clears the temp, and pushes it to the
							// - result vector
							if (temp.size() > 0) sections.push_back(temp);
							temp.clear();

							// Breaks if the end boundary is hit
							if (endBoundary) break;
							else continue;
						}
					}

					// Appends the token to the subsection
					temp += token + "\r\n";
				}

				// Starts parsing the sub sections, just like it's an normal mime body
				for (const std::string &section : sections)
					parseRecursive(section, email, ++i);
				break;
			}
			default:
			{
				// Checks if we need to perform some other
				// - procedures before ending method
				switch (parsedTransferEncoding)
				{
					case ETE_BASE64:
					{
						std::stringstream stream(body);
						std::string token;

						// Removes the newlines inside of the base64 data
						// - we want to store
						while (std::getline(stream, token, '\n'))
						{
							if (token[token.size() - 1] == '\r') token.pop_back();
							finalContent += token;
						}
						break;
					}
					default: finalContent = body;
				}
				break;
			}
		}

		// Checks if we need to save anything,
		// - we only save if there is valid content, and it's
		// - not emppty
		if (
			parsedContentType != EmailContentType::ECT_MULTIPART_ALTERNATIVE &&
			parsedContentType != EmailContentType::ECT_MULTIPART_MIXED &&
			!finalContent.empty()
		)
		{
			email.e_BodySections.push_back(EmailBodySection{
				finalContent,
				parsedContentType,
				parsedHeaders,
				static_cast<int32_t>(i),
				parsedTransferEncoding
			});
		}
	}

	/**
	 * Turns an set of email headers into a mime header set
	 */
	string buildHeaders(const vector<EmailHeader> &headers) {
		ostringstream result;

		for_each(headers.begin(), headers.end(), [&](const EmailHeader &h) {
			result << h.e_Key << ": " << h.e_Value << "\r\n";
		});

		return result.str();
	}

	/**
	 * Builds an mime header which consists of small segments
	 *  separated by a ;
	 */
	#define _BUILD_HEADER_MAX_LINE_LENGTH 73
	string buildHeader(const vector<EmailHeader> &headers) {
		ostringstream result;

		// Starts looping over the headers
		bool shouldIndent = false;
		size_t headerIndex = 0;
		for_each(headers.begin(), headers.end(), [&](const EmailHeader &h) {
			// Prepares the buffer which contains the header that should be appendend
			string buffer = h.e_Key;
			buffer += '=';
			buffer += h.e_Value;

			if (++headerIndex < headers.size()) buffer += "; ";

			// Appends the spacing if it is not the first round
			//  after which we append the buffer
			if (shouldIndent) result << "       ";
			if (buffer.size() > _BUILD_HEADER_MAX_LINE_LENGTH) {
				size_t usedLength = 0, leftLength = buffer.size();

				size_t i = 0;
				while (leftLength > 0) {
					if (++i > 1) result << "        ";
					if (leftLength > _BUILD_HEADER_MAX_LINE_LENGTH) {
						result << buffer.substr(usedLength, _BUILD_HEADER_MAX_LINE_LENGTH) << "\r\n";
						usedLength += _BUILD_HEADER_MAX_LINE_LENGTH;
						leftLength -= _BUILD_HEADER_MAX_LINE_LENGTH;
					} else {
						result << buffer.substr(usedLength) << "\r\n";
						usedLength += leftLength;
						leftLength -= leftLength;
					}
				}
			} else {
				result << buffer << "\r\n";
			}

			shouldIndent = true;
		});

		return result.str();
	}

	/**
	 * Parses an email header into k/v pairs, this will be used
	 *  for example with DKIM.
	 */
	map<string, string> subtextIntoKeyValuePairs(const string &raw) {
		map<string, string> result = {};

		stringstream stream(raw);
		string val;
		while (getline(stream, val, ';')) {
			// Cleans the first and last white, and gets the 
			//  separator index
			removeFirstAndLastWhite(val);
			size_t sepIndex = val.find_first_of('=');
			if (sepIndex == string::npos) continue;

			// Gets the key/value pair
			string key, value;
			key = val.substr(0, sepIndex);
			value = val.substr(++sepIndex);

			// Cleans the first and last whitespace from the element
			removeFirstAndLastWhite(key);
			removeFirstAndLastWhite(val);

			// Inserts the pair into the final result map
			result.insert(make_pair(key, value));
		}

		return result;
	}
}