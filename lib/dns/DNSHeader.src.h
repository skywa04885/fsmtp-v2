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
		DNS_REQ_TYPE_QUERY = 0,
		DNS_REQ_TYPE_RESPONSE
	} QueryType;

	typedef enum : uint8_t
	{
		QUERY_OP_STANDARD = 0,
		QUERY_OP_INVERSE,
		QUERY_OP_SERVER_STAT_REQ,
		QUERY_OP_INVALID
	} QueryOpcode;

	class DNSHeader
	{
	public:
		/**
		 * Gets the ID from the header
		 *
		 * @Param {char *} buffer [2 octets]
		 * @return {void}
		 */
		void getID(char *buffer);

		/**
		 * Logs the DNS header
		 *
		 * @Param {Logger &logger}
		 * @Return {void}
		 */
		void log(Logger &logger);

		/**
		 * Gets the query type
		 *
		 * @Param {void}
		 * @Return {QueryType}
		 */
		QueryType getType(void);

		/**
		 * Gets the query opcode
		 *
		 * @Param {void}
		 * @Return {QueryOpcode}
		 */
		QueryOpcode getOpcode(void);

		uint8_t d_Buffer[768];
	};
}