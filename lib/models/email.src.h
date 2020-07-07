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

#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <cstdint>

#include "../general/cleanup.src.h"
#include "../general/logger.src.h"

using namespace FSMTP::Cleanup;

namespace FSMTP::Models
{
	class EmailAddress
	{
	public:
		EmailAddress(const std::string &e_Name, const std::string &e_Address);
		EmailAddress(const std::string &raw);
		EmailAddress();

		void parse(const std::string &raw);

		void setAddress(const std::string &address);
		void setName(const std::string &name);

		void getDomain(std::string &ret);
		void getUsername(std::string &ret);

		std::string e_Address;
		std::string e_Name;
	};

	typedef enum : uint8_t
	{
		ETE_8BIT = 0,
		ETE_7BIT,
		ETE_QUOTED_PRINTABLE,
		ETE_NOT_FUCKING_KNOWN,
		ETE_NOT_FOUND
	} EmailTransferEncoding;

	typedef enum : uint8_t
	{
		ECT_TEXT_PLAIN = 0,
		ECT_TEXT_HTML,
		ECT_MULTIPART_ALTERNATIVE,
		ECT_NOT_FUCKING_KNOWN,
		ECT_NOT_FOUND
	} EmailContentType;

	/**
	 * Turns an string into an enum value
	 * - of email content type
	 *
	 * @Param {const std::string &} raw
	 * @Return {EmailContentType}
	 */
	EmailContentType stringToEmailContentType(const std::string &raw);

	/**
	 * Turns an string into an enum value of
	 * - EmailTransferEncoding type
	 *
	 * @Param {const std::string &} raw
	 * @Return {EmailContentType}
	 */
	EmailTransferEncoding stringToEmailTransferEncoding(const std::string &raw);

	typedef struct
	{
		std::string e_Key;
		std::string e_Value;
	} EmailHeader;

	typedef struct
	{
		std::string e_Content;
		EmailContentType e_Type;
		std::vector<EmailHeader> e_Headers;
		int32_t e_Index;
	} EmailBodySection;

	class FullEmail
	{
	public:
    FullEmail();

    /**
     * Prints an full email to the console
     *
     * @Param {FullEmail &} email
     * @Param {Logger &logger} logger
     * @Return {void}
     */
    static void print(FullEmail &email, Logger &logger);

    EmailContentType e_ContentType;
    EmailAddress e_TransportFrom;
		EmailAddress e_TransportTo;
		std::string e_Subject;
		std::string e_MessageID;
		std::vector<EmailBodySection> e_BodySections;
		std::vector<EmailAddress> e_From;
		std::vector<EmailAddress> e_To;
		std::vector<EmailHeader> e_Headers;
	};

}
