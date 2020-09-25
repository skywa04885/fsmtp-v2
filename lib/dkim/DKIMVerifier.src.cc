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

	/**
	 * Verifies a single signature
	 */
	DKIMVerifyResponse verifySignature(
		const string &signature, const vector<EmailHeader> &headers, 
		const string &body
	) {
		bool isBodyHashValid, isSignatureValid;
		isBodyHashValid = isSignatureValid = false;

		#ifdef _SMTP_DEBUG
		Logger logger("SIG_VERF", LoggerLevel::DEBUG);
		logger << "Verifying signature: " << signature << ENDL;
		Timer timer("Verify", logger);
		#endif

		// ==================================
		// Parses the dkim signature, and
		//  gets the values from it
		// ==================================

		// Parses the header fields from the dkim signature
		vector<tuple<string, string>> headerFields = {};
		parseDkimHeader(signature, headerFields);

		// Gets the values from the header
		DKIM::DKIMHeaderSegments parsedHeader;
		for_each(headerFields.begin(), headerFields.end(), [&](const tuple<string, string> &tup) {
			auto &key = get<0>(tup);
			auto &val = get<1>(tup);

			DEBUG_ONLY(logger << key << ": " << val << ENDL);

			if (key == "v") parsedHeader.s_Version = val.c_str();
			else if (key == "s") parsedHeader.s_KeySelector = val.c_str();
			else if (key == "d") parsedHeader.s_Domain = val.c_str();
			else if (key == "a") parsedHeader.s_Algo = val.c_str();
			else if (key == "c") parsedHeader.s_CanonAlgo = val.c_str();
			else if (key == "bh") parsedHeader.s_BodyHash = val.c_str();
			else if (key == "b") parsedHeader.s_Signature = val;
			else if (key == "h") {
				stringstream iss(val);
				string token;
				while (getline(iss, token, ':')) {
					transform(token.begin(), token.end(), token.begin(), [](const char c) {
						return tolower(c);
					});
					parsedHeader.s_Headers.push_back(token);
				}
			}
		});

		// ==================================
		// Builds the header set for the
		//  canonicalization
		// ==================================

		stringstream signatureSpecifiedHeaders;

		// Loops over the set of dkim specified headers, and attempts
		//  to find each one in the real header set. After this we
		//  append the header as an string to the signature specified headers
		//  which will be canonicalized later in this process
		for_each(parsedHeader.s_Headers.begin(), parsedHeader.s_Headers.end(), [&](const string &hKey) {
			vector<EmailHeader>::const_iterator it = find_if(headers.begin(), headers.end(), [&](const EmailHeader &h) {
				return (h.e_Key == hKey);
			});

			if (it == headers.end()) return; /*throw runtime_error(EXCEPT_DEBUG(
				string("Could not find DKIM Specified header in real headers: ") + hKey
			));*/

			signatureSpecifiedHeaders << it->e_Key << ": " << it->e_Value << "\r\n";
		});
		// for_each(headers.begin(), headers.end(), [&](const EmailHeader &h) {
		// 	vector<string>::const_iterator it = find_if(parsedHeader.s_Headers.begin(), parsedHeader.s_Headers.end(), [&](const string &str) {
		// 		cout << str << ", " << h.e_Key << endl;
		// 		return (str == h.e_Key);
		// 	});

		// 	if (it == parsedHeader.s_Headers.end()) return;

		// 	signatureSpecifiedHeaders << h.e_Key << ": " << h.e_Value << "\r\n";
		// });

		// Appends the dkim signature itself to the headers
		//  but then with the signature field left out, since
		//  this field was not known when the signing process
		//  is performed.
		signatureSpecifiedHeaders << "DKIM-SIgnature: ";
		for_each(headerFields.begin(), headerFields.end(), [&](const tuple<string, string> &tup) {
			const string &key = get<0>(tup);
			const string &val = get<1>(tup);

			if (key == "b") signatureSpecifiedHeaders << "b=";
			else signatureSpecifiedHeaders << key << '=' << val << "; ";	
		});

		// ==================================
		// Performs the canonicalization
		// ==================================

		string canonicalizedHeaders, canonicalizedBody;

		// Switches the algorithm, and attempts to perform
		//  the canonicalization
		switch (DKIM::algorithmPairFromString(parsedHeader.s_CanonAlgo)) {
			case DAP_RELAXED_RELAXED: {
				// Performs the canonicalization, and after that we remove
				//  the final <CR><LF> from the headers result, since the signature
				//  is not known, and we do not want a new line.
				canonicalizedHeaders = DKIM::_canonicalizeHeadersRelaxed(signatureSpecifiedHeaders.str(), false);
				canonicalizedBody = DKIM::_canonicalizeBodyRelaxed(body);

				canonicalizedHeaders.erase(canonicalizedHeaders.end()-2, canonicalizedHeaders.end());
				break;
			}
			default: throw runtime_error(EXCEPT_DEBUG(
				string("Algorithm pair not implemented: ") + parsedHeader.s_CanonAlgo
			));
		}

		DEBUG_ONLY(logger << "Canonicalization with " << parsedHeader.s_CanonAlgo << " finished .." << ENDL);
		DEBUG_ONLY(logger << "Canonicalized headers: " << ENDL);
		DEBUG_ONLY(cout << '\'' << canonicalizedHeaders << '\'' << endl);

		// ==================================
		// Generates and validates the body
		//  hash of the signature
		// ==================================

		cout<< canonicalizedBody << endl;
		string generatedBodyHash = Hashes::sha256base64(canonicalizedBody);

		DEBUG_ONLY(logger << "Body hash, generated: '" << generatedBodyHash << "', original: '" 
			<< parsedHeader.s_BodyHash << "'" << ENDL);

		if (generatedBodyHash == parsedHeader.s_BodyHash) isBodyHashValid = true;

		// ==================================
		// Resolves the DKIM Records
		// ==================================

		DKIM::DKIMRecord record;
		try {
			// Resolve the record
			string query = parsedHeader.s_KeySelector;
			query += '._domainkey.';
			query += parsedHeader.s_Domain;
			DEBUG_ONLY(logger << "Resolving DKIM Record for: '" << query << '\'' << ENDL);
			record = DKIM::DKIMRecord::fromDNS(query.c_str());

			// If debug enabled, print the record
			DEBUG_ONLY(record.print(logger));
		} catch (const runtime_error &e) {
			DEBUG_ONLY(logger << ERROR << "Failed to resolve/parse record: " << e.what() << ENDL << CLASSIC);
			return DKIMVerifyResponse::DVR_FAIL_RECORD;
		}

		// ==================================
		// Verifies the signature
		// ==================================

		try {		
			isSignatureValid = Hashes::RSASha256verify(parsedHeader.s_Signature, canonicalizedHeaders, record.getPublicKey());
			if (!isSignatureValid)
				DEBUG_ONLY(logger << ERROR << "Signature invalid" << ENDL);
		} catch (const runtime_error &e) {
			DEBUG_ONLY(logger << ERROR << "Failed to verify signature: " << e.what() << ENDL);
			return DKIMVerifyResponse::DVR_FAIL_SYSTEM;
		}

		// ==================================
		// Returns the according response
		// ==================================

		if (isBodyHashValid && isSignatureValid) {
			DEBUG_ONLY(logger << "Signature is valid" << ENDL);
			return DKIMVerifyResponse::DVR_PASS_BOTH;
		} else if (!isBodyHashValid && !isSignatureValid) return DKIMVerifyResponse::DVR_FAIL_BOTH;
		else if (isBodyHashValid && !isSignatureValid) return DKIMVerifyResponse::DVR_FAIL_SIGNATURE;
		else if (!isBodyHashValid && isSignatureValid) return DKIMVerifyResponse::DVR_FAIL_BODY_HASH;
		else return DKIMVerifyResponse::DVR_FAIL_SYSTEM;
	}

	/**
	 * Verifies an message which possibly contains some DKIM-Signature header
	 */
	vector<DKIMVerifyResponse> verify(const string &raw) {
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

		// ==================================
		// Gets the signatures from the headers
		// ==================================

		vector<string> signatures = {};
		for_each(headers.begin(), headers.end(), [&](const EmailHeader &h) {
			if (h.e_Key == "dkim-signature") signatures.push_back(h.e_Value);
		});

		if (signatures.size() <= 0) {
			DEBUG_ONLY(logger << "Zero signatures found in message");;
			return { DKIMVerifyResponse::DVR_FAIL_HEADER };
		}

		// ==================================
		// Starts looping over the
		//  signatures and processing them
		// ==================================		

		vector<DKIMVerifyResponse> verificationResponses = {};
		for_each(signatures.begin(), signatures.end(), [&](const string &signature) {
			try {
				verificationResponses.push_back(verifySignature(signature, headers, rawBody));
			} catch (const runtime_error &e) {
				DEBUG_ONLY(logger << ERROR << "Failed to verify signature: " << e.what() << ENDL);
				verificationResponses.push_back(DKIMVerifyResponse::DVR_FAIL_SYSTEM);
			}
		});

		return verificationResponses;
	}
}
