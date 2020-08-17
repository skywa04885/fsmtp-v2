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

namespace FSMTP::DNS {
	string resolveHostname(const char *hostname) {
		struct hostent *h = gethostbyname(hostname);
		if (h == nullptr) {
			throw runtime_error("Could not find hostname");
		}

		in_addr *address = reinterpret_cast<in_addr *>(h->h_addr);
		return inet_ntoa(*address);
	}

	vector<Record> resolveDNSRecords(
		const string &hostname,
		const RecordType type
	) {
		#ifdef _SMTP_DEBUG
		Logger logger("DNS Resolver", LoggerLevel::DEBUG);
		logger << "Vinden van records voor: " << hostname << ENDL;
		#endif

		ns_msg nsMsg;
		int32_t responseCount;
		uint8_t resultBuffer[2048];
		vector<Record> res = {};
		int rType;

		switch (type) {
			case RecordType::RT_MX:
				rType = ns_t_mx;
				break;
			case RecordType::RT_TXT:
				rType = ns_t_txt;
				break;
			default: throw runtime_error("Invalid record type");
		}

		struct __res_state state;
		res_ninit(&state);
		DEFER(res_nclose(&state));

		// Performs the query, and checks the length, if the length is lower then zero
		//  the domain was not found

		int32_t responseSize;
		if ((responseSize = res_nquery(
			&state,
			hostname.c_str(),
			ns_c_in,
			rType,
			resultBuffer,
			sizeof (resultBuffer)
		)) < 0) {
			throw runtime_error("Could not find domain");
		}

		DEBUG_ONLY(logger << "Antwoord lengte: " << responseSize << "/2048 octets" << ENDL);

		// Starts parsing the response, and looping over the entries. If the entry count
		//  is lower or equal to zero, we throw an error that no records were found

		ns_initparse(resultBuffer, responseSize, &nsMsg);
		responseCount = ns_msg_count(nsMsg, ns_s_an);
		DEBUG_ONLY(logger << "DNS Record aantal: " << responseCount << ENDL);
		if (responseCount <= 0) {
			throw runtime_error("No records found");
		}

		ns_rr record;
		char data[512];
		for (int32_t i = 0; i < responseCount; i++) {
			ns_parserr(&nsMsg, ns_s_an, i, &record);
			dn_expand(
				ns_msg_base(nsMsg),
				ns_msg_end(nsMsg),
				ns_rr_rdata(record) + 2,
				data,
				sizeof (data)
			);

			res.push_back(Record{
				(type == RecordType::RT_MX ? data : string(
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
		for (const auto &record : res) {
			logger << "DNS Record [nm: '" << record.r_Name << "', type: ";
			logger << record.r_Type << ", ttl: ";
			logger << record.r_TTL << ", class: ";
			logger << record.r_Class << ", len: ";
			logger << record.r_ReadLen << "]: " << record.r_Value << ENDL;
		}
		#endif

		return res;
	}

	string getHostnameByAddress(const struct sockaddr_in *addr) {
		char hostname[256];
    getnameinfo(
    	reinterpret_cast<const struct sockaddr *>(addr),
    	sizeof (struct sockaddr_in),
    	hostname,
    	sizeof (hostname),
    	nullptr, 0, NI_NAMEREQD
    );
    return hostname;
  }
}
