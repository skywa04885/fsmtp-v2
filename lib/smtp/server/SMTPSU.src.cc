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

#include "SMTPSU.src.h"

namespace FSMTP::Server::SU {
	/**
	 * Checks if an hostname is in the SuperUser list
	 *  this will allow to send it from fannst without
	 *  authenticating.
	 */
	bool checkSU(const string &hostname) {
		#ifdef _SMTP_DEBUG
		Logger logger("CHECKSU", LoggerLevel::DEBUG);
		logger << "Resolving SPF for: " << hostname << ENDL;
		#endif

		// Initializes the resolver state
		struct __res_state state;
		DEFER(res_nclose(&state));
		if (res_ninit(&state) < 0) {
			throw runtime_error(EXCEPT_DEBUG(
						string("Failed to initialize state: ") + strerror(errno)
			));
		}

		// Performs the query, we store the result in a 4096 byte
		//  long buffer. Should be enough i guess..
		u_char answer[4096];
		int32_t answer_len;
		if ((answer_len = res_nquery(
					&state, hostname.c_str(), 
					ns_c_in, ns_t_txt, answer, 
					sizeof(answer)
				)) < 0) return false;

		// Parses the response, and throws an exception
		//  if the response contains zero or less messages
		ns_msg msg;
		ns_initparse(answer, answer_len, &msg);
		int32_t response_count;
		if ((response_count = ns_msg_count(msg, ns_s_an)) <= 0) {
			throw runtime_error(EXCEPT_DEBUG("No records found for domain !"));
		}
		DEBUG_ONLY(logger << "Found " << response_count << " records" << ENDL);

		// Starts parsing the message, and checks for an SPF record
		ns_rr record;
		char *spf_record = nullptr;
		char record_data[2048];
		for (int32_t i = 0; i < response_count; i++) {
			ns_parserr(&msg, ns_s_an, i, &record);

			// Gets the record contents, this will also append the null term to the record's
			//  data since we will otherwise have random chars. We skip the first one too.
			char *data = reinterpret_cast<char *>(const_cast<u_char *>(ns_rr_rdata(record) + 1));
			size_t data_len = strlen(data) - 2;
			data[data_len] = '\0';

			// Checks if the record is spf
			if (data_len < 7) {
				DEBUG_ONLY(logger << "Record too short for TXT, skipping ... " << ENDL); continue;
			} else if (ns_rr_type(record) == ns_t_txt && strncmp(data, "v=spf", 5) == 0) {
				DEBUG_ONLY(logger << "SPF Record found: " << data << ENDL);
				spf_record = data;
			}
		}

		// Checks if we should parse, if no record found
		//  return false.
		if (spf_record == nullptr) return false;
		

		return true;
	}
}
