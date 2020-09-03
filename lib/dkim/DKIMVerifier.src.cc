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
		
		resolveRecord(headerSegments.s_Domain, headerSegments.s_KeySelector);

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

		//logger << "Canonicalized body: \r\n\033[41m'" << canedBody << "'\033[0m" << ENDL;
		//logger << "Canonicalized headers: \r\n\033[41m'" << canedHeaders << "'\033[0m" << ENDL;
	}

	void resolveRecord(const string &domain, const string &keySeletor) {
		string resolveFor = keySeletor;
		resolveFor += "._domainkey.";
		resolveFor += domain;

		vector<Record> records = resolveDNSRecords(resolveFor, RecordType::RT_TXT);

		for_each(records.begin(), records.end(), [&](auto &r) {
				cout << r.r_ReadLen<< endl;
		});
	}
}

