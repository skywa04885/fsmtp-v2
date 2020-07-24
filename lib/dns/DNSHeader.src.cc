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

#include "DNSHeader.src.h"

namespace FSMTP::DNS
{
	// =======================================================
	// DNS Question
	// =======================================================

	/**
	 * Default empty constructor
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	DNSQuestion::DNSQuestion(void)
	{}

	/**
	 * The parsing constructor
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	DNSQuestion::DNSQuestion(const uint8_t *parse)
	{
		this->parse(parse);
	}

	/**
	 * The parse method
	 *
	 * @Param {const char *} parse
	 * @Return {std::tuple<bool, std::size_t>}
	 */
	std::tuple<bool, std::size_t> DNSQuestion::parse(const uint8_t *parse)
	{
		const uint8_t *c = parse;
		std::size_t byteOffset = 0;

		uint8_t i = 0, iMax = *c;
		++c; ++byteOffset;

		// Loops over the data
		for (;;)
		{
			if (*c == '\0') break;

			if (i >= iMax)
			{
				iMax = *c;
				this->d_QName += '.';
				i = -1;
			} else this->d_QName += *c;

			++c;
			++i;
			++byteOffset;
		}

		// Adds one to the byte offset to skip the zero
		++byteOffset;

		// Gets the request type, and name
		this->d_QType = ntohs(*reinterpret_cast<const int16_t *>(&parse[byteOffset]));
		byteOffset += 2;
		this->d_QClass = ntohs(*reinterpret_cast<const int16_t *>(&parse[byteOffset]));

		// Returns the bool if we need to parse another question
		// - and the current byte offset of the question
		return std::tuple<bool, std::size_t>(false, byteOffset);
	}

	/**
	 * The log method
	 *
	 * @Param {Logger &logger}
	 * @Return {void}
	 */
	void DNSQuestion::log(Logger &logger)
	{
		logger << DEBUG;
		logger << "[DNS Question]: " << ENDL;
		logger << "\t- QName: " << this->d_QName << ENDL;
		logger << "\t- QClass: " << d_QClass << ENDL;
		logger << CLASSIC;
	}

	// =======================================================
	// DNS Header
	// =======================================================

	/**
	 * Default empty constructor
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	DNSHeader::DNSHeader(void):
		d_BufferULen(0)
	{}

	/**
	 * Gets the ID from the header
	 *
	 * @Param {char *} buffer [2 octets]
	 * @return {void}
	 */
	void DNSHeader::getID(char *buffer)
	{
		memcpy(buffer, this->d_Buffer, sizeof (char) * 2);
	}

	/**
	 * Gets the query type, true is query
	 *
	 * @Param {void}
	 * @Return {bool}
	 */
	bool DNSHeader::getType(void)
	{
		if (BINARY_COMPARE(this->d_Buffer[2], 0b10000000))
			return false;
		else
			return true;
	}

	/**
	 * Gets the query opcode
	 *
	 * @Param {void}
	 * @Return {QueryOpcode}
	 */
	QueryOpcode DNSHeader::getOpcode(void)
	{
		int8_t opcode = this->d_Buffer[2] & 0b01111000;
		opcode >>= 3;
		switch (opcode)
		{
			case 0: return QueryOpcode::QUERY_OP_STANDARD;
			case 1: return QueryOpcode::QUERY_OP_INVERSE;
			case 2: return QueryOpcode::QUERY_OP_SERVER_STAT_REQ;
			default: return QueryOpcode::QUERY_OP_INVALID;
		}
	}

	/**
	 * Returns the string of an query opcode
	 *
	 * @Param {const QueryOpcode} opcode
	 * @Return {const char *}
	 */
	const char *opcodeToString(const QueryOpcode opcode)
	{
		switch (opcode)
		{
			case QueryOpcode::QUERY_OP_STANDARD: return "Standard (0)";
			case QueryOpcode::QUERY_OP_INVERSE: return "Inverse (1)";
			case QueryOpcode::QUERY_OP_SERVER_STAT_REQ: return "Status Request (2)";
			case QueryOpcode::QUERY_OP_INVALID: default: return "Invalid (-1)";
		}
	}

	/**
	 * Checks if this is an authoritive answer
	 *
	 * @Param {void}
	 * @Return {bool}
	 */
	bool DNSHeader::getAA(void)
	{
		if (BINARY_COMPARE(this->d_Buffer[2], 0b00000100)) return true;
		else return false;
	}

	/**
	 * Checks if the message was truncated
	 *
	 * @Param {void}
	 * @Return {bool}
	 */
	bool DNSHeader::getTruncated(void)
	{
		if (BINARY_COMPARE(this->d_Buffer[2], 0b00000010)) return true;
		else return false;
	}

	/**
	 * Checks if recursion is desired
	 *
	 * @Param {void}
	 * @Return {bool}
	 */
	bool DNSHeader::getRecursionDesired(void)
	{
		if (BINARY_COMPARE(this->d_Buffer[2], 0b00000001)) return true;
		else return false;
	}

	/**
	 * Checks if recursion is available
	 *
	 * @Param {void}
	 * @Return {bool}
	 */
	bool DNSHeader::getRecursionAvailable(void)
	{
		if (BINARY_COMPARE(this->d_Buffer[3], 0b10000000)) return true;
		else return false;
	}

	/**
	 * Gets the response code
	 *
	 * @Param {void}
	 * @Return {ResponseCode}
	 */
	ResponseCode DNSHeader::getResponseCode(void)
	{
		int8_t responseCode =  this->d_Buffer[3] & 0b00001111;
		switch (responseCode)
		{
			case 0: return ResponseCode::QUERY_RESP_NO_ERR;
			case 1: return ResponseCode::QUERY_RESP_FORMAT_ERR;
			case 2: return ResponseCode::QUERY_RESP_SERVER_FAILURE;
			case 3: return ResponseCode::QUERY_RESP_NAME_ERROR;
			case 4: return ResponseCode::QUERY_RESP_NOT_IMPLEMENTED;
			case 5: return ResponseCode::QUERY_RESP_REFUSED;
			default: return QUERY_RESP_REFUSED;
		}
	}

	/**
	 * Returns the string version of an response code
	 *
	 * @Param {const ResponseCode} code
	 * @Return {const char *}
	 */
	const char *responseCodeString(const ResponseCode code)
	{
		switch (code)
		{
			case ResponseCode::QUERY_RESP_NO_ERR: return "No error (0)";
			case ResponseCode::QUERY_RESP_FORMAT_ERR: return "Format error (1)";
			case ResponseCode::QUERY_RESP_SERVER_FAILURE: return "Server failure (2)";
			case ResponseCode::QUERY_RESP_NAME_ERROR: return "Name error (3)";
			case ResponseCode::QUERY_RESP_NOT_IMPLEMENTED: return "Not implemented (4)";
			case ResponseCode::QUERY_RESP_REFUSED: return "Refused (5)";
			case ResponseCode::QUERY_RESP_INVALID: default: return "Invalid (-1)";
		}
	}

	/**
	 * Gets the number of questions in the question section
	 *
	 * @Param {void}
	 * @Return {uint16_t}
	 */
	uint16_t DNSHeader::getQdCount(void)
	{
		return ntohs(*reinterpret_cast<uint16_t *>(&this->d_Buffer[4]));
	}

	/**
	 * Gets the number of resource records
	 *
	 * @Param {void}
	 * @Return {uint16_t}
	 */
	uint16_t DNSHeader::getAsCount(void)
	{
		return ntohs(*reinterpret_cast<uint16_t *>(&this->d_Buffer[6]));
	}

	/**
	 * Gets the number of server resource records
	 *
	 * @Param {void}
	 * @Return {uint16_t}
	 */
	uint16_t DNSHeader::getNsCount(void)
	{
		return ntohs(*reinterpret_cast<uint16_t *>(&this->d_Buffer[8]));
	}

	/**
	 * Gets the number of additional records
	 *
	 * @Param {void}
	 * @Return {uint16_t}
	 */
	uint16_t DNSHeader::getArCount(void)
	{
		return ntohs(*reinterpret_cast<uint16_t *>(&this->d_Buffer[10]));
	}

	/**
	 * Logs the DNS header
	 *
	 * @Param {Logger &logger}
	 * @Return {void}
	 */
	void DNSHeader::log(Logger &logger)
	{
		logger << DEBUG << "[DNS Query]: " << ENDL;

		// Gets the ID, and prints the HEX encoded value
		char id[2];
	 	this->getID(id);

	 	std::string idStr;
	 	Encoding::HEX::encode(std::string(id), idStr);
	 	logger << "\t- DNS RequestID: 0x" << idStr << ENDL;

	 	// Prints the query type, request or response
	 	if (this->getType() == true)
	 		logger << "\t- DNS QueryType: Query" << ENDL;
	 	else logger << "\t- DNS QueryType: Response" << ENDL;

	 	// Prints the query operation code
	 	logger << "\t- DNS Operation: " << opcodeToString(this->getOpcode()) << ENDL;

	 	// Prints the AA, if this is an authoritive answer
	 	if (this->getAA() == true)
	 		logger << "\t- DNS AA: Authoritive" << ENDL;
	 	else logger << "\t- DNS AA: Other" << ENDL;

	 	// Prints the TC, if this message was truncated
	 	if (this->getTruncated() == true)
	 		logger << "\t- DNS TC: Truncated" << ENDL;
	 	else logger << "\t- DNS TC: Full" << ENDL;

	 	// Prints the TC, if this message was truncated
	 	if (this->getRecursionDesired() == true)
	 		logger << "\t- DNS RD: Desired ( NOT IMPLEMENTED )" << ENDL;
	 	else logger << "\t- DNS RD: Rejected" << ENDL;

	 	// Prints the TC, if this message was truncated
	 	if (this->getRecursionAvailable() == true)
	 		logger << "\t- DNS RA: Available" << ENDL;
	 	else logger << "\t- DNS RA: Not available" << ENDL;

	 	// Prints the response code
	 	logger << "\t- DNS RCODE: " << responseCodeString(this->getResponseCode()) << ENDL;

	 	// Prints the counters
	 	logger << "\t- DNS QD Count:" << this->getQdCount() << ENDL;
	 	logger << "\t- DNS AS Count:" << this->getAsCount() << ENDL;
	 	logger << "\t- DNS NS Count:" << this->getNsCount() << ENDL;
	 	logger << "\t- DNS AR Count:" << this->getArCount() << ENDL;

	 	// Sets the logger back to info, and rteturns
	 	logger << CLASSIC;
	}
}
