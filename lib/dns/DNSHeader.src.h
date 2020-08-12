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

#include "DNS.src.h"

namespace FSMTP::DNS
{
	typedef enum : uint8_t
	{
		QUERY_OP_STANDARD = 0,
		QUERY_OP_INVERSE,
		QUERY_OP_SERVER_STAT_REQ,
		QUERY_OP_INVALID
	} QueryOpcode;

	typedef enum : uint8_t
	{
		QUERY_RESP_NO_ERR = 0,
		QUERY_RESP_FORMAT_ERR,
		QUERY_RESP_SERVER_FAILURE,
		QUERY_RESP_NAME_ERROR,
		QUERY_RESP_NOT_IMPLEMENTED,
		QUERY_RESP_REFUSED,
		QUERY_RESP_INVALID
	} ResponseCode;

	typedef enum : uint8_t
	{
		QUERY_CLASS_INTERNET = 0,
		QUERY_CLASS_CSNET,
		QUERY_CLASS_CHAOS,
		QUERY_CLASS_HESIOD,
		QUERY_CLASS_NONE,
		QUERY_CLASS_ALL,
		QUERY_CLASS_UNKNOWN
	} QueryClass;

	typedef enum : uint8_t
	{
		QUERY_TYPE_QUERY = 0,
		QUERY_TYPE_IQUERY,
		QUERY_TYPE_STATUS,
		QUERY_TYPE_UNKNOWN,
		QUERY_TYPE_NOTIFY,
		QUERY_TYPE_UPDATE
	} QueryType;

	/**
	 * Turns an query class into an int16_t
	 *
	 * @Param {const QueryClass} class
	 * @Return {int16_t}
	 */
	int16_t queryClassToInt(const QueryClass c);

	/**
	 * Returns the string of an query opcode
	 *
	 * @Param {const QueryOpcode} opcode
	 * @Return {const char *}
	 */
	const char *opcodeToString(const QueryOpcode opcode);

	/**
	 * Returns the string version of an response code
	 *
	 * @Param {const ResponseCode} code
	 * @Return {const char *}
	 */
	const char *responseCodeString(const ResponseCode code);

	class DNSQuestion
	{
	public:
		/**
		 * Default empty constructor
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit DNSQuestion(void);

		/**
		 * The parsing constructor
		 *
		 * @Param {const char *} parse
		 * @Return {void}
		 */
		DNSQuestion(const uint8_t *parse);

		/**
		 * The parse method
		 *
		 * @Param {const char *} parse
		 * @Return {std::tuple<bool, std::size_t>}
		 */
		std::tuple<bool, std::size_t> parse(const uint8_t *parse);

		/**
		 * The log method
		 *
		 * @Param {Logger &logger}
		 * @Return {void}
		 */
		void log(Logger &logger);

		std::string d_QName;
		QueryClass d_QClass;
		QueryType d_QType;
	};

	class DNSHeader
	{
	public:
		/**
		 * Default empty constructor
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit DNSHeader(void);

		/**
		 * Gets the ID from the header
		 *
		 * @Param {char *} buffer [2 octets]
		 * @return {void}
		 */
		void getID(char *buffer) const;

		/**
		 * Sets the ID in the header
		 *
		 * @Param {const char *id}
		 * @Return {void}
		 */
		void setID(char *buffer);

		/**
		 * Logs the DNS header
		 *
		 * @Param {Logger &logger}
		 * @Return {void}
		 */
		void log(Logger &logger);

		/**
		 * Gets the query type, query = true
		 *
		 * @Param {void}
		 * @Return {bool}
		 */
		bool getType(void) const;

		/**
		 * Sets the query type, true is query
		 *
		 * @Param {const bool} isQuery
		 * @Return {bool}
		 */
		void setType(const bool isQuery);

		/**
		 * Gets the query opcode
		 *
		 * @Param {void}
		 * @Return {QueryOpcode}
		 */
		QueryOpcode getOpcode(void) const;

		/**
		 * Gets the query opcode
		 *
		 * @Param {const QueryOpcode} opcode
		 * @Return {void}
		 */
		void setOpcode(const QueryOpcode opcode);

		/**
		 * Checks if this is an authoritive answer
		 *
		 * @Param {void}
		 * @Return {bool}
		 */
		bool getAA(void) const;

		/**
		 * Sets this is an authoritive answer
		 *
		 * @Param {const bool} isAuthoritive
		 * @Return {void}
		 */
		void setAA(const bool isAuthoritive);

		/**
		 * Checks if the message was truncated
		 *
		 * @Param {void}
		 * @Return {bool}
		 */
		bool getTruncated(void) const;

		/**
		 * Sets the message was truncated
		 *
		 * @Param {const bool} truncated
		 * @Return {void}
		 */
		void setTruncated(const bool truncated);

		/**
		 * Checks if recursion is desired
		 *
		 * @Param {void}
		 * @Return {bool}
		 */
		bool getRecursionDesired(void) const;

		/**
		 * Checks if recursion is desired
		 *
		 * @Param {const bool} recursionDesired
		 * @Return {void}
		 */
		void setRecursionDesired(const bool recursionDesired);

		/**
		 * Checks if recursion is available
		 *
		 * @Param {void}
		 * @Return {bool}
		 */
		bool getRecursionAvailable(void) const;

		/**
		 * Sets if recursion is available
		 *
		 * @Param {const bool} recursionAvailable
		 * @Return {void}
		 */
		void setRecursionAvailable(const bool recursionAvailable);

		/**
		 * Gets the response code
		 *
		 * @Param {void}
		 * @Return {ResponseCode}
		 */
		ResponseCode getResponseCode(void) const;

		/**
		 * Sets the response code
		 *
		 * @Param {const ResponseCode rCode}
		 * @Return {void}
		 */
		void setResponseCode(const ResponseCode rCode);

		/**
		 * Gets the number of questions in the question section
		 *
		 * @Param {void}
		 * @Return {uint16_t}
		 */
		uint16_t getQdCount(void) const;

		/**
		 * Sets the QD count, number of questions
		 *
		 * @Param {const int16_t} c
		 * @Return {void}
		 */
		void setQdCount(const int16_t c);

		/**
		 * Gets the number of resource records
		 *
		 * @Param {void}
		 * @Return {uint16_t}
		 */
		uint16_t getAsCount(void) const;

		/**
		 * Sets the QAS count, the number of resource records
		 *
		 * @Param {const int16_t} c
		 * @Return {void}
		 */
		void setAsCount(const int16_t c);

		/**
		 * Gets the number of server resource records
		 *
		 * @Param {void}
		 * @Return {uint16_t}
		 */
		uint16_t getNsCount(void) const;

		/**
		 * Sets the NS count, the number of server resource records
		 *
		 * @Param {const int16_t} c
		 * @Return {void}
		 */
		void setNsCount(const int16_t c);

		/**
		 * Gets the number of additional records
		 *
		 * @Param {void}
		 * @Return {uint16_t}
		 */
		uint16_t getArCount(void) const;

		/**
		 * Sets the AR count, the number of aditional records
		 *
		 * @Param {const int16_t} c
		 * @Return {void}
		 */
		void setArCount(const int16_t c);

		/**
		 * Clones another header into the current one
		 *
		 * @Param {const DNSHeader &} header
		 * @Param {const std::size_t} to
		 * @Return {void}
		 */
		void clone(const DNSHeader &header, const std::size_t to);

		uint8_t d_Buffer[768];
		int32_t d_BufferULen;
	};
}