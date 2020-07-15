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

#include "DNS.src.h"

namespace FSMTP::DNS
{
	/**
	 * Resolves the IP Address from an hostname
	 *
	 * @Param {const char *} hostname
	 * @Return {std::string}
	 */
	std::string resolveHostname(const char *hostname)
	{
		// Gets the hostent and checks if it is there
		// - if not throw domain not found error
		struct hostent *h = gethostbyname(hostname);
		if (h == nullptr)
			throw std::runtime_error("Could not find hostname");

		// Parses the address from the hostent
		in_addr *address = reinterpret_cast<in_addr *>(h->h_addr);
		return inet_ntoa(*address);
	}

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
	)
	{
		#ifdef _SMTP_DEBUG
		Logger logger("DNS Resolver", LoggerLevel::DEBUG);
		logger << "Vinden van records voor: " << hostname << ENDL;
		#endif

		uint8_t resultBuffer[2048];
		std::vector<Record> res = {};
		int rType;

		// =================================
		// Performs the query
		//
		// Checks the record type we want
		// - and performs the query
		// =================================

		switch (type)
		{
			case RecordType::RT_MX:
			{
				rType = ns_t_mx;
				break;
			}
			case RecordType::RT_TXT:
			{
				rType = ns_t_txt;
				break;
			}
			default: throw std::runtime_error("Invalid record type");
		}

		// Initializes the state
		struct __res_state state;
		res_ninit(&state);

		// Performs the query, and checks if the query
		// - could be performed
		int32_t responseSize = res_nquery(
			&state,
			hostname.c_str(),
			ns_c_in,
			rType,
			resultBuffer,
			sizeof (resultBuffer)
		);

		if (responseSize < 0)
		{
			res_nclose(&state);
			throw std::runtime_error("Could not find domain");
		}

		DEBUG_ONLY(logger << "Antwoord lengte: " << responseSize << "/2048 octets" << ENDL);

		// =================================
		// Gets the data
		//
		// Parses the query results and
		// - stores it inside the vector
		// =================================
		ns_msg nsMsg;
		int32_t responseCount;

		// Initializes the parser, checks the count
		// - and throws error if there was nothing
		ns_initparse(resultBuffer, responseSize, &nsMsg);
		responseCount = ns_msg_count(nsMsg, ns_s_an);
		if (responseCount < 0)
		{
			res_nclose(&state);
			throw std::runtime_error("No records found");
		}

		DEBUG_ONLY(logger << "DNS Record aantal: " << responseCount << ENDL);

		// Loops over the records and parses them
		ns_rr record;
		char data[512];
		for (int32_t i = 0; i < responseCount; i++)
		{
			// Parses the record and gets the contents
			ns_parserr(&nsMsg, ns_s_an, i, &record);
			dn_expand(
				ns_msg_base(nsMsg),
				ns_msg_end(nsMsg),
				ns_rr_rdata(record) + 2,
				data,
				sizeof (data)
			);

			// Pushes the data to the result vector
			res.push_back(Record{
				(type == RecordType::RT_MX ? data : std::string(
					reinterpret_cast<char *>(const_cast<unsigned char *>(ns_rr_rdata(record))),
					ns_rr_rdlen(record)
				)),
				ns_rr_name(record),
				ns_rr_type(record),
				static_cast<int32_t>(ns_rr_ttl(record)),
				ns_rr_class(record),
				ns_rr_rdlen(record)
			});
		}

		#ifdef _SMTP_DEBUG
		for (const Record &record : res)
		{
			logger << "DNS Record [nm: '" << record.r_Name << "', type: "
				<< record.r_Type << ", ttl: "
				<< record.r_TTL << ", class: "
				<< record.r_Class << ", len: "
				<< record.r_ReadLen << "]: " << record.r_Value << ENDL;
		}
		#endif

		// Frees the memory and returns the result
		res_nclose(&state);
		return res;
	}

	/**
	 * Gets the hostname based on the ip address
	 *
	 * @Param {struct sockaddr_in *addr} addr
	 * @Return {std::string}
	 */
	std::string getHostnameByAddress(struct sockaddr_in *addr)
	{
		char hostname[256];
    getnameinfo(
    	reinterpret_cast<struct sockaddr *>(addr),
    	sizeof (struct sockaddr_in),
    	hostname,
    	sizeof (hostname),
    	NULL, 0, NI_NAMEREQD
    );
    return hostname;
  }
}
