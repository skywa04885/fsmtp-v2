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
	DKIMAlgorithmPair algorithmPairFromString(const string &compare) {
		if (compare == "relaxed/relaxed")
			return DKIMAlgorithmPair::DAP_RELAXED_RELAXED;
		else if (compare == "relaxed/simple")
			return DKIMAlgorithmPair::DAP_RELAXED_SIMPLE;
		else if (compare == "simple/simple")
			return DKIMAlgorithmPair::DAP_SIMPLE_SIMPLE;
		else if (compare == "simple/relaxed")
			return DKIMAlgorithmPair::DAP_SIMPLE_RELAXED;
		else throw runtime_error("Algorithm not recognized");
	}

	
	/**
	 * Signs an raw email using the DKIM algorithm
	 *
	 * @Param {const std::string&} email
	 * @Param {const DKIMConfig &} config
	 * @Return {std::string}
	 */
	std::string sign(const std::string &email, const DKIMConfig &config)
	{
		#ifdef _SMTP_DEBUG
		Logger logger("DKIMSigner", LoggerLevel::DEBUG);
		Timer timer("Signer", logger);
		#endif

		DKIMHeaderSegments segments;
		std::string headers, body;

		// Adds the known variables to the target signature
		segments.s_Version = "1";
		segments.s_Domain = config.c_Domain;
		segments.s_KeySelector = config.c_KeySelector;
		segments.s_Algo = "rsa-sha256";

		// ========================================
		// Parses the message
		//
		// Gets the headers, body and lists the
		// - final set of headers we  will use
		// ========================================

		// Separates the headers and body, and stores
		// - it inside the strings
		MIME::splitHeadersAndBody(email, headers, body);

		// Prepares the parsable headers
		std::string headersToParse = headers;
		MIME::joinMessageLines(headersToParse);

		// Parses the headers and gets the keys, then
		// - we push the keys to the segment headers
		std::vector<EmailHeader> parsedHeaders = {};
		MIME::parseHeaders(headersToParse, parsedHeaders, true);
		for (const EmailHeader &h : parsedHeaders)
		{
			if (shouldUseHeader(h.e_Key))
				segments.s_Headers.push_back(h.e_Key);
		}

		// ========================================
		// Canonicalizes the message
		//
		// Canonicalizes the headers and body
		// ========================================

		std::string cannedBody, cannedHeaders;

		DEBUG_ONLY(logger << "Started canonicalization of headers and body" << ENDL);

		switch (config.c_Algo)
		{
			case DKIMAlgorithmPair::DAP_RELAXED_RELAXED:
			{
				// Canonicalizes
				cannedBody = _canonicalizeBodyRelaxed(body);
				cannedHeaders = _canonicalizeHeadersRelaxed(headers);

				// Sets the segment canon algorithm
				segments.s_CanonAlgo = "relaxed/relaxed";
				break;
			}
			default: throw std::runtime_error(EXCEPT_DEBUG("Algorithm pair not implemented"));
		}

		DEBUG_ONLY(logger << "Canonicalized headers: \r\n\033[41m'" << cannedHeaders.substr(0, cannedHeaders.size() - 2) << "'\033[0m" << ENDL);
		DEBUG_ONLY(logger << "Canonicalized body: \r\n\033[41m'" << cannedBody.substr(0, cannedBody.size() - 2) << "'\033[0m" << ENDL);

		// ========================================
		// Prepares headers
		//
		// Prepares the headers for the final sign
		// - ing process 
		// ========================================

		// Hashes the body, ands puts the body hash inside of the
		// - segments, so we can generate the body
		segments.s_BodyHash = Hashes::sha256base64(cannedBody);
		DEBUG_ONLY(logger << "Generated body hash: " << segments.s_BodyHash << ENDL);

		// Generates the first signature without line breaks
		// - and without the final hash, then we append them
		// - to the headers and create the signature
		std::string preSignatureheader = buildDKIMHeader(segments, false);
		cannedHeaders += "dkim-signature:" + preSignatureheader;

		// ========================================
		// Prepares headers
		//
		// Generates the signature and creates the
		// - final set of headers
		// ========================================

		// Creates the signature
		DEBUG_ONLY(logger << "Raw signature: \r\n\033[41m'" << cannedHeaders << "'\033[0m" << ENDL);
		segments.s_Signature = Hashes::RSAShagenerateSignature(
			cannedHeaders, 
			config.c_PrivateKeyPath,
			EVP_sha256()
		);
		DEBUG_ONLY(logger << "Generated signature: " << segments.s_Signature << ENDL);

		// Generates the new set of header key values, and
		// - appends it to the final email
		std::string dkimSignature = buildDKIMHeader(segments, true);
		DEBUG_ONLY(logger << "Final DKIM Signature: \r\n\033[41mDKIM-Signature: " << dkimSignature << "\033[0m" << ENDL);

		// Generates the signed message
		std::string res = headers;
		res += "DKIM-Signature: " + dkimSignature + "\r\n\r\n";
		res += body;

		return res;
	}

	/**
	 * Canonicalizes an message body with the relaxed algorithm
	 *
	 * rewritten at: Fri sep 4
	 */
	string _canonicalizeBodyRelaxed(const string &raw) {
		string reduced;
		string res;

		// Reduces the whitespace
		reduced.reserve(raw.length());
		reduceWhitespace(raw, reduced);

		// Splits the document up into empty lines, so we can
		//  later process them more easily
		list<string> lines = {};
		stringstream stream(reduced);
		string line;
		while (getline(stream, line, '\n')) {
			if (!line.empty() && line[line.size() - 1] == '\r') line.pop_back();
			lines.push_back(line);
		}

		// Trims the extra whitespace, and removes the dots at the beginning of lines
		size_t i = 0;
		size_t lastLineWithContent = 0;
		for_each(lines.begin(), lines.end(), [&](string &line) {
			// Removes the whitespace at the end of the line
			line.erase(line.find_last_not_of(' ') + 1);
			if (line[0] == '.' && line[1] == '.') line.erase(0, 1);

			// Checks if the line is empty, if so
			//  just continue, else report it as the
			//  last full line
			if (!line.empty()) lastLineWithContent = i;
			i++;
		});

		// Removes the empty lines at the end of the message
		i = 0;
		for (list<string>::iterator it = lines.begin(); it != lines.end();) {
			if (i++ > lastLineWithContent) {
				lines.erase(it++);
				continue;
			}
			it++;
		}

		// Joins all the lines into one message,
		//  which will be stored in res
		for_each(lines.begin(), lines.end(), [&](string &line) {
			res += line;
			res += "\r\n";
		});
		return res;
	}

	/**
	 * Canonicalizes the headers using the relaxed algorithm
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string _canonicalizeHeadersRelaxed(const std::string &raw, bool signingCheck)
	{
		std::string res;

		// Creates the string stream and starts looping
		// - over all the headers, ignoring empty lines
		std::stringstream stream(raw);
		std::string token;
		while (std::getline(stream, token, '\n'))
		{
			std::string key, value;

			// Removes the '\r' if it is there, after that
			// - we check if the line is empty, and if so ignore
			// - it
			if (!token.empty() && token[token.size() - 1] == '\r') token.pop_back();
			if (token.empty()) continue;

			// Finds the key and value and processes them,
			// - first we find the separator
			std::size_t sepIndex = token.find_first_of(':');
			if (sepIndex == std::string::npos) continue;

			// Separates the key and value, after that
			// - we remove all non required whitespace
			reduceWhitespace(token.substr(0, sepIndex), key);
			reduceWhitespace(token.substr(sepIndex), value);
			removeFirstAndLastWhite(key);
			if (value[0] == ':') value.erase(0, 1);
			if (value[0] == ' ') value.erase(0, 1);

			// Makes the key lowercase
			std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){
				return std::tolower(c);
			});

			// Checks if we should use the header
			if (signingCheck && !shouldUseHeader(key)) continue;

			// Constructs the header, and pushes it back to
			// - the res
			res += key + ':' + value + "\r\n";
		}

		return res;
	}

	/**
	 * Checks if we should use an specified header
	 *
	 * @Param {const std::string &} key
	 * @Return {bool}
	 */
	bool shouldUseHeader(const std::string &key)
	{
		if (key == "subject")
			return true;
		if (key == "message-id")
			return true;
		if (key == "date")
			return true;
		if (key == "from")
			return true;
		if (key == "to")
			return true;
		if (key == "mime-version")
			return true;
		if (key == "dkim-signature")
			return true;
		return false;
	}

	/**
	 * Buids the DKIM header based on the segments, auto format
	 * - tells the function if we need to break the lines and format
	 * - it transport compatable
	 *
	 * @Param {const DKIMHeaderSegments &} segments
	 * @Param {bool} autoFormat
	 * @Return {std::string}
	 */
	std::string buildDKIMHeader(const DKIMHeaderSegments &segments, bool autoFormat)
	{
		std::string res;

		// Merges all the headers into one stringstream
		std::ostringstream headersImploded;
		std::copy(
			segments.s_Headers.begin(),
			segments.s_Headers.end(),
			std::ostream_iterator<std::string>(headersImploded, ":")
		);
		std::string headers = headersImploded.str();
		headers.pop_back();

		// Creates the key-value pairmap
		std::map<const char *, std::string> pairs = {
			std::make_pair("v", segments.s_Version),
			std::make_pair("a", segments.s_Algo),
			std::make_pair("c", segments.s_CanonAlgo),
			std::make_pair("d", segments.s_Domain),
			std::make_pair("s", segments.s_KeySelector),
			std::make_pair("h", headers),
			std::make_pair("bh", segments.s_BodyHash),
			std::make_pair("b", segments.s_Signature)
		};

		if (autoFormat)
		{
			std::size_t lineLength = 0;
			bool firstLine = true;
			for (
				std::map<const char *, std::string>::iterator it = pairs.begin();
				it != pairs.end(); it++
			)
			{
				// Creates the key value pair we should append, and
				// - checks if it will fit, if not we will do something
				// - about it
				std::string shouldAppend = it->first;
				shouldAppend += "=";
				shouldAppend += it->second;
				shouldAppend += "; ";

				if (lineLength + shouldAppend.size() >= 98)
				{
					// Checks if it would fit if we added an new
					// - line, else we will separate it over multiple
					// - lines
					if (shouldAppend.size() < 98)
					{
						firstLine = false;
						res += "\r\n       "; // ( 7 spaces )
						res += shouldAppend;
						lineLength = shouldAppend.size() + 7;
					} else
					{
						// Starts adding them over multiple lines
						for (const char c : shouldAppend)
						{
							if (++lineLength >= 98)
							{
								res += "\r\n        "; // ( 8 spaces )
								lineLength = 8;
							}

							res += c;
						}
					}
				} else
				{
					res += shouldAppend;
					lineLength += shouldAppend.size();
				}
			}

			res.pop_back();
			res.pop_back();
		} else
		{
			for (
				std::map<const char *, std::string>::iterator it = pairs.begin();
				it != pairs.end(); it++
			)
			{
				res += it->first;
				res += "=";
				res += it->second;
				res += "; ";
			}

			// Removes the last two chars, ( the space and semicolon )
			res.pop_back();
			res.pop_back();
		}

		return res;
	}
}
