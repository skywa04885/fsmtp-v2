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

#include "DKIMVerifier.src.h"
#include <csignal>
#include <openssl/ssl3.h>

namespace FSMTP::DKIM_Verifier {
	void parseDkimHeader(const string &raw, map<string, string> &header) {
		stringstream ss(raw);
		string pair;
		
		while (getline(ss, pair, ';')) {
			size_t sep = pair.find_first_of('=');
			if (!sep)
				throw runtime_error("Invalid DKIM header");

			string key = pair.substr(0, sep);
			string value = pair.substr(sep + 1);

			Cleanup::removeFirstAndLastWhite(key);
			Cleanup::removeFirstAndLastWhite(value);

			header.insert(make_pair(move(key), move(value)));
		}
	}

	bool verify(const string &raw) {
		Logger logger("DKIMVerifier", LoggerLevel::DEBUG);

		// Splits the headers from the body, after which
		//  we parse the headers
		string rawHeaders, rawBody;
		MIME::splitHeadersAndBody(raw, rawHeaders, rawBody);

		vector<EmailHeader> headers = {};
		MIME::parseHeaders(rawHeaders, headers, false);

		// Checks if the DKIM header is there, if not
		//  we just return false, else we parse the values from
		//  the DKIM header
		map<string, string> dkimHeader; // IMPORTANT ! Keep in scope !
		bool dkimHeaderFound = false;
		for (const auto &header : headers) {
			if (header.e_Key == "dkim-signature") {
				parseDkimHeader(header.e_Value, dkimHeader);

				dkimHeaderFound = true;
				break;
			}
		}

		if (!dkimHeaderFound) {
			logger << "Could not find DKIM-Signature !" << ENDL;
			return false;
		}
		logger << "Found DKIM-Signature" << ENDL;

		// Starts getting the values from the DKIM header
		DKIM::DKIMHeaderSegments headerSegments;
		map<string, string>::iterator it;
		for (it = dkimHeader.begin(); it != dkimHeader.end(); ++it) {
			logger << it->first << ": " << it->second << ENDL;

			auto &k = it->first;
			auto &v = it->second;
			if (k == "v") {
				// Version
				headerSegments.s_Version = v.c_str();
			} else if (k == "s") {
				// KeySelector
				headerSegments.s_KeySelector = v.c_str();
			} else if (k == "d") {
				// Domain
				headerSegments.s_Domain = v.c_str();
			} else if (k == "a") {
				// Algorithm
				headerSegments.s_Algo = v.c_str();
			} else if (k == "c") {
				// CanonicalizationAlgorithm
				headerSegments.s_CanonAlgo = v.c_str();
			} else if (k == "bh") {
				// BodyHash
				headerSegments.s_BodyHash = v.c_str();
			} else if (k == "b") {
				// Signature
				headerSegments.s_Signature = move(v);
			} else if (k == "h") {
				// The header selection
				stringstream iss(v);
				string token;
				while (getline(iss, token, ':')) {
					headerSegments.s_Headers.push_back(token);
				}
			}
		}

		// Gets the public key from the DNS TXT record, this
		//  is required to decode the signature
		
		string record = resolveRecord(headerSegments.s_Domain, headerSegments.s_KeySelector);
		DEBUG_ONLY(logger << "DKIM Record: \r\n\033[44m'" << record << "'\033[0m" << ENDL);
		map<string, string> dkimValueMap = MIME::subtextIntoKeyValuePairs(record);

		if (dkimValueMap.count("p") <= 0) throw runtime_error(EXCEPT_DEBUG("Public key not found !"));

		// Since we now know which headers were used by the signer
		//  we now want to get them ourselves, so we can canonicalize
		//  them
		
		ostringstream finalHeaders;
		auto &segHeaders = headerSegments.s_Headers;
		for_each(headers.begin(), headers.end(), [&](const auto &header) {
			if (find(segHeaders.begin(), segHeaders.end(), header.e_Key) != segHeaders.end())
				finalHeaders << header.e_Key << ": " << header.e_Value << "\r\n";
		});

		DKIM::DKIMAlgorithmPair algorithm = DKIM::algorithmPairFromString(headerSegments.s_CanonAlgo);
		
		// Starts canonicalizing the body and headers, this is done
		//  using the above specified algorithm
		
		string canedBody, canedHeaders;
		switch (algorithm) {
			case DKIM::DKIMAlgorithmPair::DAP_RELAXED_RELAXED:
				canedBody = DKIM::_canonicalizeBodyRelaxed(rawBody);
				canedHeaders = DKIM::_canonicalizeHeadersRelaxed(finalHeaders.str());
				break;
			default: throw runtime_error("Algorithm not implemented");
		}

		DEBUG_ONLY(logger << "Canonicalized body: \r\n\033[44m'" << canedBody << "'\033[0m" << ENDL);
		DEBUG_ONLY(logger << "Canonicalized headers: \r\n\033[44m'" << canedHeaders << "'\033[0m" << ENDL);

		// Performs the hash of the headers, and the hash of the body. After this we will use
		//  openssl to decode the signature with the public key. Then we compare both hashes
		//  and return the result

		string bodyHash = Hashes::sha256base64(canedBody);
		DEBUG_ONLY(logger << "Body hash: " << bodyHash << ENDL);

		Hashes::RSASha256verify(headerSegments.s_Signature, canedHeaders, dkimValueMap.find("p")->second);
	}

	string resolveRecord(const string &domain, const string &keySeletor) {
		#ifdef _SMTP_DEBUG
		Logger logger("DKIM_RESOLV", LoggerLevel::DEBUG);
		#endif
		string resolveFor = keySeletor;
		resolveFor += "._domainkey.";
		resolveFor += domain;

		struct __res_state state;
		if (res_ninit(&state) < 0) {
			throw runtime_error(EXCEPT_DEBUG(
				string("Failed to initialize resolver state: ") + strerror(errno)
			));
		}
		DEFER(res_nclose(&state));

		// Performs the query for the DKIM Record
		u_char answer[8096];
		int32_t answer_len;
		if ((answer_len = res_nquery(&state, resolveFor.c_str(), 
				ns_c_in, ns_t_txt, answer, 
				sizeof (answer))) <= 0) {
			throw runtime_error(EXCEPT_DEBUG(
				string("Failed to resolve records: ") + strerror(errno)
			));
		}

		// Parses the response, and gets the number of records
		//  which later will be checked for DKIM
		ns_msg msg;
		int32_t record_count;
		if ((ns_initparse(answer, answer_len, &msg)) < 0) {
			throw runtime_error(EXCEPT_DEBUG(
				string("Failed to initialize parser: ") + strerror(errno)
			));
		}

		if ((record_count = ns_msg_count(msg, ns_s_an)) <= 0) {
			throw runtime_error(EXCEPT_DEBUG("DKIM Record not found"));
		}

		// Starts looping over the records, until a valid DKIM record has been found
		//  we then proceed by parsing it
		ns_rr record;
		string dkim_record;
		for (int32_t i = 0; i < record_count; ++i) {
			if (ns_parserr(&msg, ns_s_an, i, &record)) {
				throw runtime_error(EXCEPT_DEBUG("Failed to parse record"));
			}

			// Gets the text value from the dkm record, and prints
			//  the content if we're in debug mode
			string rdata = string(reinterpret_cast<const char *>(ns_rr_rdata(record) + 1), ns_rr_rdlen(record) - 1);
			DEBUG_ONLY(logger << "Checking record: " << rdata << ENDL);

			// Checks if the record is actually DKIM Like, else just continue
			if (rdata.substr(0, 7) != "v=DKIM1") continue;

			// Filters away the non-ascii crap, since this indicates the switch between UDP
			//  and TCP
			dkim_record.reserve(rdata.length());
			for (unsigned char c : rdata) {
				if (c >= 0 && c < 128) dkim_record += c;
			}
		}

		// Returns the resolved record
		if (dkim_record.empty()) throw runtime_error(EXCEPT_DEBUG("No record found !"));
		else return dkim_record;
	}
}

