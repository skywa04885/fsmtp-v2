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

#include <vector>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <cstring>

#include <resolv.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "../general/logger.src.h"
#include "../general/macros.src.h"

namespace FSMTP::DNS
{
	typedef enum : uint8_t
	{
		RT_MX = 0,
		RT_TXT
	} RecordType;

	typedef struct
	{
		std::string r_Value;
		std::string r_Name;
		int32_t r_Type;
		int32_t r_TTL;
		int32_t r_Class;
		int32_t r_ReadLen;
	} Record;

	/**
	 * Resolves the IP Address from an hostname
	 *
	 * @Param {const char *} hostname
	 * @Return {const char *}
	 */
	std::string resolveHostname(const char *hostname);

	/**
	 * Resolves an set of records
	 *
	 * @Param {std::string &} hostname
	 * @Param {RecordType} type
	 * @Return {std::vector<Record>}
	 */
	std::vector<Record> resolveDNSRecords(
		const std::string &hostname,
		const RecordType type
	);

	/**
	 * Gets the hostname based on the ip address
	 *
	 * @Param {struct sockaddr_in *addr} addr
	 * @Return {std::string}
	 */
	std::string getHostnameByAddress(struct sockaddr_in *addr);
}
