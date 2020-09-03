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

#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <stdexcept>

#include "../general/Logger.src.h"
#include "../general/cleanup.src.h"
#include "../models/Email.src.h"
#include "../general/encoding.src.h"

using namespace FSMTP::Cleanup;
using namespace FSMTP::Models;
using namespace FSMTP::Encoding;

namespace FSMTP::Parsers::MIME
{
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
	);

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
	);

	/**
	 * Joins separated lines inside of the messae
	 * these are some or another way required ;(
	 *
	 * @Parma {std::string &} raw
	 */
	void joinMessageLines(
		std::string &raw
	);

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
	);

	/**
	 * Parses header sub arguments
	 *
	 * @Param {const std::string &} raw
	 * @Return {void}
	 */
	std::vector<std::string> parseHeaderSubtext(const std::string &raw);

	/**
	 * Parses the value of an argument like a="asd"
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string parseSubtextValue(const std::string &raw);

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
	);

	/**
	 * Turns an set of email headers into a mime header set
	 */
	string buildHeaders(const vector<EmailHeader> &headers);

	/**
	 * Builds an mime header which consists of small segments
	 *  separated by a ;
	 */
	string buildHeader(const vector<EmailHeader> &headers);
}