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
	void parseDkimHeader(const string &raw, vector<tuple<string, string>> &header) {
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

			header.push_back(tuple<string, string>(key, value));
		}
	}

	DKIMVerifyResponse verify(const string &raw) {
		bool signatureValid, bodyHashValid;
		DEBUG_ONLY(Logger logger("DKIMVerifier", LoggerLevel::DEBUG));

		// ==================================
		// Parses the mime message
		// ==================================

		// Splits the body into lines, so we can parse the
		//  headers from it
		vector<string> lines = getMIMELines(raw);

		// Splits the headers from the body, after which
		//  we parse the headers
		strvec_it headersStart, headersEnd, bodyStart, bodyEnd;
		tie(headersStart, headersEnd, bodyStart, bodyEnd) = splitMIMEBodyAndHeaders(lines.begin(), lines.end());
		vector<EmailHeader> headers = _parseHeaders(headersStart, headersEnd, true);

		// Gets the raw body from the splitted lines, this is required
		//  for signing
		string rawBody = getStringFromLines(bodyStart, bodyEnd);

		// Checks if the DKIM header is there, if not
		//  we just return false, else we parse the values from
		//  the DKIM header
		vector<tuple<string, string>> dkimHeader;
		const string *rawDKIMHeader = nullptr;
		bool dkimHeaderFound = false;
		for (auto &header : headers) {
			auto &val = header.e_Value;
			auto &key = header.e_Key;
			transform(key.begin(), key.end(), key.begin(), [](const char c){
				return tolower(c);
			});

			if (key == "dkim-signature") {
				parseDkimHeader(val, dkimHeader);
				rawDKIMHeader = &val;

				dkimHeaderFound = true;
				break;
			}
		}

		if (!dkimHeaderFound) {
			DEBUG_ONLY(logger << "Could not find DKIM-Signature !" << ENDL);
			return DKIMVerifyResponse::DVR_FAIL_HEADER;
		}
		DEBUG_ONLY(logger << "Found DKIM-Signature" << ENDL);

		// ==================================
		// Gets the values from the header
		// ==================================

		// Starts getting the values from the DKIM header
		DKIM::DKIMHeaderSegments headerSegments;
		vector<tuple<string, string>>::iterator it;
		for (it = dkimHeader.begin(); it != dkimHeader.end(); ++it) {
			DEBUG_ONLY(logger << get<0>(*it) << ": " << get<1>(*it) << ENDL);

			auto &k = get<0>(*it);
			auto &v = get<1>(*it);
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
				headerSegments.s_Signature = v;
			} else if (k == "h") {
				// The header selection
				stringstream iss(v);
				string token;
				while (getline(iss, token, ':')) {
					transform(token.begin(), token.end(), token.begin(), [](const char c) {
						return tolower(c);
					});
					headerSegments.s_Headers.push_back(token);
				}
			}
		}

		// ==================================
		// Verifies the signature
		// ==================================

		// Gets the public key from the DNS TXT record, this
		//  is required to decode the signature

		string record;
		try {
			record = resolveRecord(headerSegments.s_Domain, headerSegments.s_KeySelector);
		} catch (const runtime_error &e) {
			return DKIMVerifyResponse::DVR_FAIL_RECORD;
		}

		DEBUG_ONLY(logger << "DKIM Record: \r\n\033[44m'" << record << "'\033[0m" << ENDL);
		map<string, string> dkimValueMap = MIME::subtextIntoKeyValuePairs(record);

		// Checks if the public key is actually in the record
		//  if not return fail record error
		if (dkimValueMap.count("p") <= 0) return DKIMVerifyResponse::DVR_FAIL_RECORD;
		auto &publicKey = dkimValueMap.find("p")->second;

		// Since we now know which headers were used by the signer
		//  we now want to get them ourselves, so we can canonicalize
		//  them
		
		ostringstream finalHeaders;
		auto &segHeaders = headerSegments.s_Headers;
		// for_each(headers.begin(), headers.end(), [&](const EmailHeader &header) {
		// 	if (find(segHeaders.begin(), segHeaders.end(), header.e_Key) != segHeaders.end())
		// 		finalHeaders << header.e_Key << ": " << header.e_Value << "\r\n";
		// });
		for_each(segHeaders.begin(), segHeaders.end(), [&](const string &a) {
			vector<EmailHeader>::iterator header = find_if(headers.begin(), headers.end(), [&](const EmailHeader &h) {
				return (h.e_Key == a);
			});
			if (header == headers.end()) {
				throw runtime_error(EXCEPT_DEBUG("DKIM Specified header not found in headers"));
			}

			finalHeaders << header->e_Key << ": " << header->e_Value << "\r\n";
		});

		// Adds the DKIM-SIgnature to the final headers, this is always required
		//  for the signing process, so we need to add them in order to verify

		finalHeaders << "DKIM-Signature: ";
		for_each(dkimHeader.begin(), dkimHeader.end(), [&](const tuple<string, string> tup) {
			auto &key = get<0>(tup);
			auto value = get<1>(tup);

			if (key == "b") {
				finalHeaders << "b=";
				return;
			} else if (key == "h") {
				transform(value.begin(), value.end(), value.begin(), [](const char c) {
					return tolower(c);
				});
			}

			finalHeaders << key << '=' << value << "; ";
		});

		// Starts canonicalizing the body and headers, this is done
		//  using the above specified algorithm

		DKIM::DKIMAlgorithmPair algorithm = DKIM::algorithmPairFromString(headerSegments.s_CanonAlgo);

		string canedBody, canedHeaders;
		switch (algorithm) {
			case DKIM::DKIMAlgorithmPair::DAP_RELAXED_RELAXED: {
				canedBody = DKIM::_canonicalizeBodyRelaxed(rawBody);
				canedHeaders = DKIM::_canonicalizeHeadersRelaxed(finalHeaders.str());

				// Removes the CRLF because we included the signature this time
				canedHeaders.erase(canedHeaders.size() - 2);
				break;
			}
			default: throw runtime_error("Algorithm not implemented");
		}

		DEBUG_ONLY(logger << "Canonicalized body: \r\n\033[44m'" << canedBody << "'\033[0m" << ENDL);
		DEBUG_ONLY(logger << "Canonicalized headers: \r\n\033[44m'" << canedHeaders << "'\033[0m" << ENDL);

		// Creates the body hash, and compares it against the body
		//  hash specified in the email message

		string bodyHash = Hashes::sha256base64(canedBody);
		DEBUG_ONLY(logger << "Body hash: " << bodyHash << ", in message: " << headerSegments.s_BodyHash << ENDL);
		
		if (bodyHash == headerSegments.s_BodyHash) {
			DEBUG_ONLY(logger << "Body hashes do match" << ENDL);
			bodyHashValid = true;
		} else {
			DEBUG_ONLY(logger << "Body hashes do not match" << ENDL);
			bodyHashValid = false;
		}

		// Verifies the signature, against the one specified in the message
		//  this also makes use of the public key

		try {		
			signatureValid = Hashes::RSASha256verify(headerSegments.s_Signature, canedHeaders, publicKey);
		} catch (const runtime_error &e) {
			DEBUG_ONLY(logger << ERROR << "Failed to verify signature: " << e.what() << ENDL);
			return DKIMVerifyResponse::DVR_FAIL_SYSTEM;
		}

		// ==================================
		// Returns the validation response
		// ==================================

		if (bodyHashValid && signatureValid) {
			DEBUG_ONLY(logger << "Response: Body hash and signature valid" << ENDL);
			return DKIMVerifyResponse::DVR_PASS_BOTH;
		} else if (!bodyHashValid && !signatureValid) {
			DEBUG_ONLY(logger << "Response: Body hash and signature invalid" << ENDL);
			return DKIMVerifyResponse::DVR_FAIL_BOTH;
		} else if (bodyHashValid && !signatureValid) {
			DEBUG_ONLY(logger << "Response: Invalid signature" << ENDL);
			return DKIMVerifyResponse::DVR_FAIL_SIGNATURE;
		} else if (!bodyHashValid && signatureValid) {
			DEBUG_ONLY(logger << "Response: invalid body hash" << ENDL);
			return DKIMVerifyResponse::DVR_FAIL_BODY_HASH;
		} else {
			DEBUG_ONLY(logger << "Respose: System failure" << ENDL);
			return DKIMVerifyResponse::DVR_FAIL_SYSTEM;
		}
	}

	string resolveRecord(const string &domain, const string &keySeletor) {		
		string resolveFor = keySeletor;
		resolveFor += "._domainkey.";
		resolveFor += domain;

		#ifdef _SMTP_DEBUG
		Logger logger("DKIM_RESOLV", LoggerLevel::DEBUG);
		logger << "Resolving for: " << resolveFor << ENDL;
		#endif

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
			if (rdata.substr(0, 7) != "v=DKIM1" && rdata.substr(0, 5) != "k=rsa") continue;

			// Filters away the non-ascii crap, since this indicates the switch between UDP
			//  and TCP
			dkim_record.reserve(rdata.length());
			for (unsigned char c : rdata) {
				if (c >= 32 && c <= 126) dkim_record += c;
			}
		}

		// Returns the resolved record
		if (dkim_record.empty()) throw runtime_error(EXCEPT_DEBUG("No record found !"));
		else return dkim_record;
	}
}

