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
	bool checkSU(const string &hostname, const string &checkAddr) {
		#ifdef _SMTP_DEBUG
		Logger logger("CHECKSU", LoggerLevel::DEBUG);
		logger << "SU Checker by Luke A.C.A. Rieff" << ENDL;
		logger << "Resolving SPF for: " << hostname << ENDL;
		#endif

		// Initializes the resolver state
		struct __res_state state;
		if (res_ninit(&state) < 0) {
			throw runtime_error(EXCEPT_DEBUG(
						string("Failed to initialize state: ") + strerror(errno)
			));
		}
		DEFER(res_nclose(&state));

		// Performs the query, we store the result in a 4096 byte
		//  long buffer. Should be enough i guess..
		u_char answer[8192];
		int32_t answer_len;
		if ((answer_len = res_nquery(
					&state, hostname.c_str(), 
					ns_c_in, ns_t_txt, answer, 
					sizeof(answer)
				)) < 0) return false;

		// Parses the response, and throws an exception
		//  if the response contains zero or less messages
		ns_msg msg;
		if (ns_initparse(answer, answer_len, &msg)) {
			throw runtime_error(string("Failed to initialize parser: ") + strerror(errno));
		}

		int32_t response_count;
		if ((response_count = ns_msg_count(msg, ns_s_an)) <= 0) {
			throw runtime_error(EXCEPT_DEBUG("No records found for domain !"));
		}
		DEBUG_ONLY(logger << "Found " << response_count << " records" << ENDL);

		// Starts parsing the message, and checks for an SPF record
		// cout << string((const char *)answer, answer_len) << endl;
		ns_rr record;
		string spf_record;
		for (int32_t i = 0; i < response_count; ++i) {
			if (ns_parserr(&msg, ns_s_an, i, &record)) {
				throw runtime_error(string("Failed to parse record: ") + strerror(errno));
			}

			// Gets the data from the SPF record
			spf_record = string(reinterpret_cast<const char *>(ns_rr_rdata(record) + 1), ns_rr_rdlen(record) - 1);
			DEBUG_ONLY(logger << "Checking record: " << spf_record << ENDL);

			// Checks if the record is spf
			if (spf_record.length() < 7) {
				DEBUG_ONLY(logger << "Record too short for TXT, skipping ... " << ENDL); continue;
			} else if (ns_rr_type(record) == ns_t_txt && strncmp(spf_record.c_str(), "v=spf", 5) == 0) {
				DEBUG_ONLY(logger << "SPF Record found: " << spf_record << ENDL);
				break;
			}
		}

		// Checks if we should parse, if no record found
		//  return false.
		if (spf_record.empty()) {
			DEBUG_ONLY(logger << "No valid record found !" << ENDL);
			return false;
		}

		DNS::SPF::SPFRecord spf(spf_record);
		DEBUG_ONLY(spf.print(logger));

		// Checks if the SPF record allows all outgoing stuff
		if (spf.getAllowNoQuestionsAsked()) {
			DEBUG_ONLY(logger << "All addresses allowed, or deprecated record" << ENDL);
			return true;
		}
		
		// Checks if we should recurse, if we recurse we return
		//  the result from the recursion
		if (spf.shouldRedirect()) {
			DEBUG_ONLY(logger << "Redirect required, started recurse: " 
					<< spf.getRedirectURI() << ENDL);
			return checkSU(spf.getRedirectURI(), checkAddr);
		}

		// Checks if the current address is in the allowed addresses,
		//  else we resolve the MX or A records if allowed is specified
		auto &ipv4s = spf.getAllowedIPV4s();
		for (auto &addr : ipv4s) {
			if (addr_compare(checkAddr, addr, AddrType::AT_IPv4)) return true;
		}

		// Checks if the MX records are allowed addresses, if so
		//  we perform an query and check all the addresses
		if (spf.getMXRecordsAllowed()) {
			DEBUG_ONLY(logger << "MX Allowed, resolving .." << ENDL);

			vector<string> mxRecords = getMXAddresses(hostname);
			DEBUG_ONLY(logger << "MX Record addresses found: " << ENDL);
			DEBUG_ONLY({
				size_t i = 0;
				for_each(mxRecords.begin(), mxRecords.end(), [&](auto &r) {
					logger << '\t' << i++ << ": " << r << ENDL;
				});
			});

			// Checks if one of the email addresses matches the
			//  address specified by the app
			for (auto &r : mxRecords) if (addr_compare(checkAddr, r, AddrType::AT_IPv4)) return true;
		}

		// Checks if A records are allowed, if so
		//  start looping over them, and checking if in there
		if (spf.getARecordsAllowed()) {
			DEBUG_ONLY(logger << "A Allowed, resolving .." << ENDL);

			vector<string> aRecords = getARecordAddresses(hostname);
			DEBUG_ONLY(logger << "A Record addresses found: " << ENDL);
			DEBUG_ONLY({
				size_t i = 0;
				for_each(aRecords.begin(), aRecords.end(), [&](auto &r) {
					logger << '\t' << i++ << ": " << r << ENDL;
				});
			});

			// Checks if one of the email addresses matches the
			//  address specified by the app
			for (auto &r : aRecords) if (addr_compare(checkAddr, r, AddrType::AT_IPv4)) return true;
		}

		auto &allowed_domains = spf.getAllowedDomains();
		if (allowed_domains.size() > 0) {
			for (auto &domain : allowed_domains) {
				if (checkSU(domain, checkAddr)) return true;
			}
		}

		DEBUG_ONLY(logger << checkAddr << " is not an SU for " << hostname << ENDL); 
		return false;
	}


	vector<string> getMXAddresses(const string &hostname) {
		struct __res_state state;
		if (res_ninit(&state) < 0) {
			throw runtime_error(EXCEPT_DEBUG(
				string("Failed to initialize state: ") + strerror(errno)
			));
		}
		DEFER(res_nclose(&state));

		// Performs the query for the MX records
		u_char answer[8192];
		int32_t answer_len;
		if ((answer_len = res_nquery(
				&state, hostname.c_str(), 
				ns_c_in, ns_t_mx, answer, 
				sizeof(answer)
			)) < 0) return {};


		// Parses the response message and counts
		//  the amount of records
		ns_msg msg;
		ns_initparse(answer, answer_len, &msg);
		int response_count;
		if ((response_count = ns_msg_count(msg, ns_s_an)) <= 0) return {};

		// Loops over all the records, and turns them into
		//  ip addresses
		ns_rr record;
		vector<string> records = {};
		records.reserve(response_count);
		char data[1024];
		for (int32_t i = 0; i < response_count; ++i) {
			ns_parserr(&msg, ns_s_an, i, &record);
			dn_expand(ns_msg_base(msg), ns_msg_end(msg),
				ns_rr_rdata(record) + 2, data,
				sizeof (data));

			// Turns the hostname in to an ip address
			//  which can be used for comparison later on
			struct hostent *h;
			if ((h = gethostbyname(data)) == nullptr) continue;
			in_addr *address = reinterpret_cast<in_addr *>(h->h_addr);
			records.push_back(inet_ntoa(*address));
		}

		return records;
	}

	vector<string> getARecordAddresses(const string &hostname) {
		struct hostent *h;
		if ((h = gethostbyname(hostname.c_str())) == nullptr) return {};

		// Gets all the 'A' records, and parses the ip
		//  addresses from them.
		vector<string> result = {};
		struct in_addr **p = reinterpret_cast<struct in_addr **>(h->h_addr_list);
		for (; *p != nullptr; p++) result.push_back(inet_ntoa(**p));
		return result;
	}
}
