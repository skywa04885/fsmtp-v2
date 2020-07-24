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
	 * Gets the query type
	 *
	 * @Param {void}
	 * @Return {QueryType}
	 */
	QueryType DNSHeader::getType(void)
	{
		if (BINARY_COMPARE(this->d_Buffer[2], 0b1000000))
			return QueryType::DNS_REQ_TYPE_RESPONSE;
		else
			return QueryType::DNS_REQ_TYPE_QUERY;
	}

	/**
	 * Gets the query opcode
	 *
	 * @Param {void}
	 * @Return {QueryOpcode}
	 */
	QueryOpcode getOpcode(void)
	{

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
	 	if (this->getType() == QueryType::DNS_REQ_TYPE_QUERY)
	 		logger << "\t- DNS QueryType: Query" << ENDL;
	 	else
	 		logger << "\t- DNS QueryType: Response" << ENDL;

	 	// Sets the logger back to info, and rteturns
	 	logger << CLASSIC;
	}
}
