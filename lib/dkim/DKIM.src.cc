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

#include "DKIM.src.h"

namespace FSMTP::DKIM
{
	/**
	 * Signs an raw email using the DKIM algorithm
	 *
	 * @Param {const std::string&} email
	 * @Param {const DKIMConfig &} config
	 * @Return {void}
	 */
	void sign(const std::string &email, const DKIMConfig &config)
	{
		#ifdef _SMTP_DEBUG
		Logger logger("DKIMSigner", LoggerLevel::DEBUG);
		Timer timer("Signer", logger);
		#endif

		std::string headers, body;
		std::vector<EmailHeader> finalHeaders = {};

		// ========================================
		// Parses the message
		//
		// Gets the headers, body and lists the
		// - final set of headers we  will use
		// ========================================

		// Separates the headers and body, and stores
		// - it inside the strings
		MIME::splitHeadersAndBody(email, headers, body);

		// Parses the headers and gets the keys
		std::vector<EmailHeader> parsedHeaders = {};
		MIME::parseHeaders(headers, parsedHeaders, true);
		for (const EmailHeader &h : parsedHeaders)
		{
			if (h.e_Key == "subject")
				finalHeaders.push_back(h);
			if (h.e_Key == "message-id")
				finalHeaders.push_back(h);
			if (h.e_Key == "date")
				finalHeaders.push_back(h);
			if (h.e_Key == "from")
				finalHeaders.push_back(h);
			if (h.e_Key == "to")
				finalHeaders.push_back(h);
			if (h.e_Key == "mime-version")
				finalHeaders.push_back(h);
		}

		// ========================================
		// Canonicalizes the message
		//
		// Canonicalizes the headers and body
		// ========================================
		std::string relaxedBody, relaxedHeaders;

		DEBUG_ONLY(logger << "Started canonicalization of headers and body" << ENDL);

		switch (config.c_Algo)
		{
			case DKIMAlgorithmPair::DAP_RELAXED_RELAXED:
			{
				relaxedBody = _canonicalizeBodyRelaxed(body);
			}
			default: throw std::runtime_error("Algorithm pair not implemented");
		}
	}

	/**
	 * canonicalizes the body using the relaxed algorithm
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string _canonicalizeBodyRelaxed(const std::string &raw)
	{
		std::string temp;
		std::string res;

		// Reduces the whitespace occurences
		reduceWhitespace(raw, temp);

		// Removes the whitespace at start and end of lines
		// - using an string stream
		std::stringstream stream(raw);
		std::string token;
		while (std::getline(stream, token, '\n'))
		{
			// Removes the '\r' if it is there
			if (!token.empty() && token[token.size() - 1] == '\r') token.pop_back();

			// Checks if it is an empty line, if so ignore it
			if (token.empty()) continue;

			// Removes the whitespace at start and end, and pushes it to
			// - the result
			removeFirstAndLastWhite(token);
			res += token + "\r\n";
		}

		return res;
	}

	/**
	 * Canonicalizes the headers using the relaxed algorithm
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string _canonicalizeHeadersRelaxed(const std::string &raw)
	{
		std::string res;

		// Creates the string stream and starts looping
		// - over all the headers, ignoring empty lines
		std::stringstream stream(raw);
		std::string token;
		while (std::getline(stream, token, '\n'))
		{
			
		}

		return res;
	}
}