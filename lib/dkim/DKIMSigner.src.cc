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

#include "DKIMSigner.src.h"

namespace FSMTP::DKIM {
	DKIMSigner::DKIMSigner():
		m_Logger("DKIMSigner", LoggerLevel::DEBUG)
	{}

	DKIMSigner &DKIMSigner::setDomain(const string &domain)
	{ this->m_Config.domain = domain; return *this; }

	DKIMSigner &DKIMSigner::setKeySelector(const string &keySelector)
	{ this->m_Config.keySelector = keySelector; return *this; }
	
	DKIMSigner &DKIMSigner::setPrivateKeyPath(const string &privateKeyPath)
	{ this->m_Config.privateKeyPath = privateKeyPath; return *this; }
	
	DKIMSigner &DKIMSigner::setSignTime(int64_t signTime)
	{ this->m_Config.signTime = signTime; return *this; }
	
	DKIMSigner &DKIMSigner::setExpireTime(int64_t expireTime)
	{ this->m_Config.expireTime = expireTime; return *this; }
	
	DKIMSigner &DKIMSigner::setAlgoPair(DKIMHeaderCanonAlgPair algorithmPair)
	{ this->m_Config.algorithmPair = algorithmPair; return *this; }
	
	DKIMSigner &DKIMSigner::setSignAlgo(DKIMHeaderAlgorithm signAlgorithm)
	{ this->m_Config.signAlgorithm = signAlgorithm; return *this; }

	DKIMSigner &DKIMSigner::setConfig(const DKIMSignerConfig &config)
	{ this->m_Config = config; return *this; }

	DKIMSigner &DKIMSigner::setHeaderFilter(const vector<string> &filter)
	{ this->m_Config.headerFilter = filter; return *this; }

	DKIMSigner &DKIMSigner::headerFilterPush(const string &header)
	{ this->m_Config.headerFilter.push_back(header); return *this; }

	DKIMSigner &DKIMSigner::sign(const string &mime) {
		DEBUG_ONLY(auto &logger = this->m_Logger);

		// ==============================
		// Sets result values
		// ==============================

		// Sets the default values such as the headers
		this->m_Result.setKeySelector(this->m_Config.keySelector)
			.setDomain(this->m_Config.domain)
			.setHeaders(this->m_Config.headerFilter)
			.setHeaderAlgo(this->m_Config.signAlgorithm)
			.setCanonAlgoPair(this->m_Config.algorithmPair);

		// Since the result headers have been set, we can now insert
		//  the dkim record in the allowed headers, since we do not want
		//  it in the final header, but do want it in the signature
		this->m_Config.headerFilter.push_back("dkim-signature");

		// ==============================
		// Parses the MIME message
		// ==============================

		// Splits the MIME message into lines so we can process
		//  them later more easily
		vector<string> lines = Parsers::getMIMELines(mime);

		// Gets the header and body from the lines, these will
		//  be stored as iterators
		strvec_it headersBegin, headersEnd, bodyBegin, bodyEnd;
		tie(headersBegin, headersEnd, bodyBegin, 
			bodyEnd) = Parsers::splitMIMEBodyAndHeaders(lines.begin(), lines.end());

		// ==============================
		// Generates the body hash
		// ==============================

		// Generates the body hash with the current canonicalization algorithm
		//  and signing algorithm
		this->generateBodyHash(Parsers::getStringFromLines(bodyBegin, bodyEnd));

		// ==============================
		// Generates the fake signature
		// ==============================

		// Generates the pre-sign signature, which will contain everything
		//  except the signature, this will be appended to the headers to be
		//  canonicalized
		string presignSignature = this->m_Result.build();

		// Prints the presign signature to the debug console
		//  just to inform that everything is going fine
		DEBUG_ONLY(logger << "Generated presign signature: '" << presignSignature << '\'' << ENDL);

		// Appends the presign signature to the final headers
		//  so we can start signing them
		string headers = Parsers::getStringFromLines(headersBegin, headersEnd);
		headers += presignSignature;

		// ==============================
		// Generates the signature
		// ==============================

		// Generates the signature based on the algorithm
		//  specified in the config
		this->generateSignature(headers);

		// ==============================
		// Builds the signed message
		// ==============================

		this->m_SignedMessage = Parsers::getStringFromLines(headersBegin, headersEnd);
		this->m_SignedMessage += Builders::foldHeader(this->m_Result.build(), 128);
		this->m_SignedMessage += "\r\n\r\n";
		this->m_SignedMessage += Parsers::getStringFromLines(bodyBegin, bodyEnd);

		return *this;
	}

	void DKIMSigner::generateBodyHash(const string &body) {
		DEBUG_ONLY(auto &logger = this->m_Logger);

		// Generates the canonicalized body based on the algorithm
		//  specified by the implementation
		string canonicalizedBody;
		switch (this->m_Config.algorithmPair) {
			case DKIMHeaderCanonAlgPair::RelaxedRelaxed:
			case DKIMHeaderCanonAlgPair::SimpleRelaxed:
				DEBUG_ONLY(logger << "Processing body with relaxed canonicalization" << ENDL);
				canonicalizedBody = relaxedBody(body);
				break;
			case DKIMHeaderCanonAlgPair::RelaxedSimple:
			case DKIMHeaderCanonAlgPair::SimpleSimple:
				DEBUG_ONLY(logger << "Processing body with simple canonicalization" << ENDL);
				canonicalizedBody = simpleBody(body);
				break;
		}

		// Performs the hasing of the body, this is just the body hash
		//  so we either use sha1 or sha256
		switch (this->m_Config.signAlgorithm) {
			case DKIMHeaderAlgorithm::HeaderAlgorithmRSA_SHA1:
				DEBUG_ONLY(logger << "Generating body-hash with RSA-SHA1" << ENDL);
				this->m_Result.setBodyHash(Hashes::sha1base64(canonicalizedBody));
				break;
			case DKIMHeaderAlgorithm::HeaderAlgoritmRSA_SHA256:
				DEBUG_ONLY(logger << "Generating body-hash with RSA-SHA256" << ENDL);
				this->m_Result.setBodyHash(Hashes::sha256base64(canonicalizedBody));
				break;
		}

	}

	void DKIMSigner::generateSignature(const string &headers) {
		DEBUG_ONLY(auto &logger = this->m_Logger);
		auto &hf = this->m_Config.headerFilter;

		// Performs the header canonicalization with the specified algorithm
		//  we support all ;)
		string canonicalizedHeaders;
		switch (this->m_Config.algorithmPair) {
			case DKIMHeaderCanonAlgPair::RelaxedRelaxed:
			case DKIMHeaderCanonAlgPair::RelaxedSimple:
				DEBUG_ONLY(logger << "Processing headers with relaxed canonicalization" << ENDL);
				canonicalizedHeaders = relaxedHeaders(headers, hf);
				break;
			case DKIMHeaderCanonAlgPair::SimpleRelaxed:
			case DKIMHeaderCanonAlgPair::SimpleSimple:
				DEBUG_ONLY(logger << "Processing headers with simple canonicalization" << ENDL);
				canonicalizedHeaders = simpleHeaders(headers, hf);
				break;
		}

		// Removes the last endline from the canonicalized headers, since the
		//  signature is not complete
		canonicalizedHeaders.erase(canonicalizedHeaders.end() - 2, canonicalizedHeaders.end());

		// Prints the debug message with the canonicalized headers
		//  inside of them
		#ifdef _SMTP_DEBUG
		logger << "Canonicalized headers: '" << FLUSH;
		cout << canonicalizedHeaders;
		cout << '\'' << endl;
		#endif

		// Performs the signing of the canonicalized headers, this will be
		//  set as the final signature in the header
		switch (this->m_Config.signAlgorithm) {
			case DKIMHeaderAlgorithm::HeaderAlgorithmRSA_SHA1:
				DEBUG_ONLY(logger << "Generating signature with RSA-SHA1" << ENDL);
				this->m_Result.setSignature(Hashes::RSAShagenerateSignature(canonicalizedHeaders,
					this->m_Config.privateKeyPath.c_str(), EVP_sha1())); break;
			case DKIMHeaderAlgorithm::HeaderAlgoritmRSA_SHA256:
				DEBUG_ONLY(logger << "Generating signature with RSA-SHA256" << ENDL);
				this->m_Result.setSignature(Hashes::RSAShagenerateSignature(canonicalizedHeaders,
					this->m_Config.privateKeyPath.c_str(), EVP_sha256())); break;
		}
	}

	const string &DKIMSigner::getResult() const
	{ return this->m_SignedMessage; }

	DKIMSigner::~DKIMSigner() = default;
}
