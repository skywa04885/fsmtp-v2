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

#include <cassandra.h>

#include "../general/cleanup.src.h"
#include "../general/logger.src.h"
#include "../general/connections.src.h"
#include "../general/exceptions.src.h"

using namespace FSMTP::Cleanup;

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
	class EmailAddress
	{
	public:
		/**
		 * Variable constructor for the EmailAddres
		 *
		 * @Param {const std::string &} e_Name
		 * @Param {const std::string &} e_Address
		 * @Return {void}
		 */
		EmailAddress(const std::string &e_Name, const std::string &e_Address);
		
		/**
		 * Parse constructor for the email address
		 * - basically calls EmailAddress::parse()
		 *
		 * @Param {const std::string &} raw
		 * @Return {void}
		 */
		explicit EmailAddress(const std::string &raw);

		/**
		 * The empty constructor for the EmailAddress
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit EmailAddress();

		/**
		 * Parses an raw string into an EmailAddress
		 *
		 * @Param {const std::string &} raw
		 * @Return {void}
		 */
		void parse(const std::string &raw);

		/**
		 * Parses the domain name from the address
		 *
		 * @Param {std::string &} ret
		 * @Return {void}
		 */
		void getDomain(std::string &ret);

		/**
		 * parses the username from the address
		 *
		 * @Param {std::string &} ret
		 * @Return {void}
		 */
		void getUsername(std::string &ret);

		/**
		 * Parses an raw string into multiple addresses
		 *
		 * @Param {const std::string &} raw
		 * @Return {std::vector<EmailAddress>}
		 */
		static std::vector<EmailAddress> parseAddressList(const std::string &raw);

		/**
		 * Turns an vector of addresses into an string
		 *
		 * @Param {const std::vector<EmailAddress> &} addresses
		 * @Return {std::string}
		 */
		static std::string addressListToString(const std::vector<EmailAddress> &addresses);

		std::string e_Address;
		std::string e_Name;
	};

	typedef enum : uint8_t
	{
		ET_INCOMMING = 0,
		ET_INCOMMING_SPAM,
		ET_OUTGOING,
		ET_RELAY_OUTGOING,
	} EmailType;

	typedef enum : uint8_t
	{
		ETE_8BIT = 0,
		ETE_7BIT,
		ETE_BASE64,
		ETE_QUOTED_PRINTABLE,
		ETE_NOT_FUCKING_KNOWN,
		ETE_NOT_FOUND
	} EmailTransferEncoding;

	typedef enum : uint8_t
	{
		ECT_TEXT_PLAIN = 0,
		ECT_TEXT_HTML,
		ECT_MULTIPART_ALTERNATIVE,
		ECT_MULTIPART_MIXED,
		ECT_NOT_FUCKING_KNOWN,
		ECT_NOT_FOUND,
		ECT_IMAGE_PNG,
		ECT_IMAGE_JPG,
		ECT_IMAGE_GIF,
		ECT_FILE_PDF,
		ECT_FILE_OTHER
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
	 * Turns an enum value into an string
	 *
	 * @Param {const EmailContentType} type
	 * @Return {const char *}
	 */
	const char *contentTypeToString(const EmailContentType type);

	/**
	 * Turns an enum into an string
	 *
	 * @Param {const EmailTransferEncoding} enc
	 * @Return {const char *}
	 */
	const char *contentTransferEncodingToString(const EmailTransferEncoding enc);

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
		EmailTransferEncoding e_TransferEncoding;
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

    /**
     * Gets the current message bucket, basically
     * - the current time in milliseconds / 1000 / 1000 / 1000
     *
     * @Param {void}
     * @Return {int64_t}
     */
    static int64_t getBucket(void);

    /**
     * Saves an email into the database
     *
     * @Param {std::unique_ptr<CassandraConnection> &} conn
     * @Return {void}
     */
    void save(std::unique_ptr<CassandraConnection> &conn);

    EmailAddress e_TransportFrom;
		EmailAddress e_TransportTo;
		std::string e_Subject;
		std::string e_MessageID;
		std::vector<EmailBodySection> e_BodySections;
		std::vector<EmailAddress> e_From;
		std::vector<EmailAddress> e_To;
		std::vector<EmailHeader> e_Headers;

		bool e_Encryped;
		std::size_t e_Date;
		int64_t e_Bucket;
		std::string e_OwnersDomain;
		CassUuid e_OwnersUUID;
		CassUuid e_EmailUUID;
		EmailType e_Type;
	};

}
