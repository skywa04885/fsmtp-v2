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
#include "DNSHeader.src.h"

namespace FSMTP::DNS
{
	typedef enum : uint8_t
	{
		REC_TYPE_A = 0,
		REC_TYPE_AAAA,
		REC_TYPE_MX,
		REC_TYPE_SOA,
		REC_TYPE_TXT,
		REC_TYPE_UNKNOWN
	} ResponseRecordType;

	/**
	 * Turns an response record type back into an integer
	 *
	 * @Param {const ResponseRecordType} type
	 * @Return {int16_t}
	 */
	int16_t responseRecordTypeToInt(const ResponseRecordType type);

	class DNSRecord
	{
	public:
		/**
		 * Default empty constructor of an record
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit DNSRecord(void);

		/**
		 * The constructor for an record
		 *
		 * @Param {const std::string &} r_Data
		 * @Param {const std::string &} r_Root
		 * @Param {const int32_t} r_TTL,
		 * @Param {const ResponseRecordType} r_Type
		 * @Param {const QueryClass r_Class}
		 * @Return {void}
		 */
		DNSRecord(const std::string &r_Data, const std::string &r_Root,
			const int32_t r_TTL, const ResponseRecordType r_Type, 
			const QueryClass r_Class);

		/**
		 * Builds an response record
		 *
		 * @Param {char *} ret
		 * @Return {std::size_t}
		 */
		std::size_t build(char *ret) const;

		std::string r_Data;
		std::string r_Root;
		int32_t r_TTL;
		ResponseRecordType r_Type;
		QueryClass r_Class;
	};

	class Domain
	{
	public:
		/**
		 * Default empty constructor for domain
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit Domain(void);

		/**
		 * Logs the domain to the console
		 *
		 * @Param {Logger &} logger
		 * @Return {void}
		 */
		void log(Logger &logger);

		std::string d_Domain;
		std::vector<DNSRecord> d_ARecords;
	};

	/**
	 * Reads the zone from the config
	 *
	 * @Param {void}
	 * @Return {std::vector<Domain>}
	 */
	std::vector<Domain> readConfig(void);

	class Zone
	{
	public:
	private:
	};
}