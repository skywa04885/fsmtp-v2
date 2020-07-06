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
		DEBUG_ONLY(Logger logger("MIME Header Parser", LoggerLevel::DEBUG));

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
		DEBUG_ONLY(Logger logger("MIME Message Parser", LoggerLevel::DEBUG));

		// Reduces whitespace to one occurance and
		// - Joins the lines
		std::string newRaw;
		reduceWhitespace(raw, newRaw);
		raw = newRaw;
		joinMessageLines(raw);

		// Splits the headers and body into two
		// - separate strings
		std::string body, headers;
		DEBUG_ONLY(logger << "Begonnen met het opsplitsen van de headers en de body ..." << ENDL);
		splitHeadersAndBody(raw, headers, body);

		// Parses the headers into an vector
		DEBUG_ONLY(logger << "Begonnen met het opsplitsen van de headers, en rotzooi van microsoft te verwijderen ..." << ENDL);
		parseHeaders(headers, email.e_Headers, removeMsGarbage);

		// Finds the content type and boundary,
		// - then parses the body
		for (EmailHeader &h : email.e_Headers)
		{
			if (h.e_Key == "content-type")
			{
				// Parses the content type values,
				// - after that we check which content type it is
				std::vector<std::string> vals = parseHeaderSubtext(h.e_Value);
				email.e_ContentType = stringToEmailContentType(vals[0]);
				
				if (email.e_ContentType == EmailContentType::ECT_NOT_FUCKING_KNOWN)
					throw std::runtime_error("Content type is invalid, or not implemented.");

				// Checks the body type to implement parsing method
				switch (email.e_ContentType)
				{
					case EmailContentType::ECT_MULTIPART_ALTERNATIVE:
					{
						// Checks if there is an boundary, if so get it
						// - and start the parsing process
						if (vals.size() < 2)
							throw std::runtime_error("Multipart body requires an boundary supplied in the 'Content-Type' header.");

						// Parses the boundary from the content type header
						// - of course we perform strict syntax checking
						std::size_t ind = vals[1].find_first_of('"');
						if (ind == std::string::npos)
							throw std::runtime_error("Content-Type header contains invalid boundary");

						std::string boundary = vals[1].substr(++ind);
						ind = boundary.find_last_of('"');
						if (ind == std::string::npos)
							throw std::runtime_error("Content-Type header contains invalid boundary");
						boundary = boundary.substr(0, ind);

						// Processes the multipart body
						parseMultipartBody(body, email.e_ContentType, email.e_BodySections, boundary);
						break;
					}
				}
			}
		}

		// parseMultipartBody(body, )
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
			case EmailContentType::ECT_MULTIPART_ALTERNATIVE:
			{
				// ======================================================
				// Start multipart/alternative parser
				// ======================================================

				// Starts 
				std::stringstream stream(raw);
				std::string token, sectionTemp;
				std::size_t contentIndex;
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

							// Parses the section temp into valid content
							// - first starting with the headers
							std::vector<EmailHeader> headersParsed;
							std::string headers;
							std::string body;
							splitHeadersAndBody(sectionTemp, headers, body);

							// Parses the headers
							parseHeaders(headers, headersParsed, false);

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
}