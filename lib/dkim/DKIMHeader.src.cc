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

#include "DKIMHeader.src.h"

namespace FSMTP::DKIM {
	const char *__dkimHeaderVersionToString(DKIMHeaderVersion v) {
		switch (v) {
			case DKIMHeaderVersion::HeaderVersionDKIM1: return "1";
		}
	}

	const char *__dkimHeaderCanonAlgPairToString(DKIMHeaderCanonAlgPair a) {
		switch (a) {
			case DKIMHeaderCanonAlgPair::RelaxedRelaxed: return "relaxed/relaxed";
			case DKIMHeaderCanonAlgPair::RelaxedSimple: return "relaxed/simple";
			case DKIMHeaderCanonAlgPair::SimpleRelaxed: return "simple/relaxed";
			case DKIMHeaderCanonAlgPair::SimpleSimple: return "simple/simple";
		}
	}

	const char *__dkimHeaderAlgToString(DKIMHeaderAlgorithm a) {
		switch (a) {
			case DKIMHeaderAlgorithm::HeaderAlgorithmRSA_SHA1: return "rsa-sha1";
			case DKIMHeaderAlgorithm::HeaderAlgoritmRSA_SHA256: return "rsa-sha256";
		}
	}

	DKIMHeader::DKIMHeader():
		m_Version(DKIMHeaderVersion::HeaderVersionDKIM1),
		m_CanonAlgoPair(DKIMHeaderCanonAlgPair::RelaxedRelaxed),
		m_HeaderAlgorithm(DKIMHeaderAlgorithm::HeaderAlgoritmRSA_SHA256),
		m_ExpireDate(0), m_SignDate(0)
	{}

	DKIMHeader &DKIMHeader::parse(const string &raw) {
		// =================================
		// Splits the header into segments
		// =================================

		vector<string> segments = {};
		size_t start = 0, end = raw.find_first_of(';');
		for (;;) {
			segments.push_back(raw.substr(start, end - start));
			if (end == string::npos) break;

			start = end + 1;
			end = raw.find_first_of(';', start);
		}

		// =================================
		// Starts making sense of the segs
		// =================================

		auto parseHeaders = [&](string val) {
			transform(val.begin(), val.end(), val.begin(), [](const char c) { return tolower(c); });
			start = 0, end = val.find_first_of(':');
			for (;;) {
				this->m_Headers.push_back(val.substr(start, end - start));
				if (end == string::npos) break;

				start = end + 1;
				end = val.find_first_of(':', start);
			}
		};

		// Loops over all the segments and makes sense of them
		for_each(segments.begin(), segments.end(), [&](const string &seg) {
			if (seg.empty()) return;

			// Splits the segment in to a key value pair
			size_t sep = seg.find_first_of('=');
			if (sep == string::npos)
				throw runtime_error(EXCEPT_DEBUG("Could not parse k/v pair of: '" + seg + '\''));

			string key = seg.substr(0, sep), val = seg.substr(++sep);
			if (*key.begin() == ' ') key.erase(key.begin(), key.begin() + 1);
			if (*(key.end() - 1) == ' ') key.pop_back();
			transform(key.begin(), key.end(), key.begin(), [](const char c) { return tolower(c); });

			if (*val.begin() == ' ') val.erase(key.begin(), key.begin() + 1);
			if (*(val.end() - 1) == ' ') val.pop_back();

			// Checks the key, and parses / stores the value of it
			//  inside the current class
			if (key == "v") { // The version 
				if (val == "1") this->m_Version = DKIMHeaderVersion::HeaderVersionDKIM1;
				else this->m_Version = DKIMHeaderVersion::HeaderVersionDKIM1;
			} else if (key == "s") { // Selector of public key
				this->m_KeySelector = val;
			} else if (key == "d") { // The domain of the signer
				this->m_Domain = val;
			} else if (key == "c") { // The canonicalization algorithm
				transform(val.begin(), val.end(), val.begin(), [](const char c) { return tolower(c); });
				if (val == "relaxed/relaxed") this->m_CanonAlgoPair = DKIMHeaderCanonAlgPair::RelaxedRelaxed;
				else if (val == "relaxed/simple") this->m_CanonAlgoPair = DKIMHeaderCanonAlgPair::RelaxedSimple;
				else if (val == "simple/relaxed") this->m_CanonAlgoPair = DKIMHeaderCanonAlgPair::SimpleRelaxed;
				else if (val == "simple/simple") this->m_CanonAlgoPair = DKIMHeaderCanonAlgPair::SimpleSimple;
				else this->m_CanonAlgoPair = DKIMHeaderCanonAlgPair::RelaxedRelaxed;
			} else if (key == "a") { // The algorithm
				transform(val.begin(), val.end(), val.begin(), [](const char c) { return tolower(c); });
				if (val == "rsa-sha256") this->m_HeaderAlgorithm = DKIMHeaderAlgorithm::HeaderAlgoritmRSA_SHA256;
				else if (val == "rsa-sha1") this->m_HeaderAlgorithm = DKIMHeaderAlgorithm::HeaderAlgorithmRSA_SHA1;
				else this->m_HeaderAlgorithm = DKIMHeaderAlgorithm::HeaderAlgoritmRSA_SHA256;
			} else if (key == "bh") { // The body-hash
				this->m_BodyHash = val;
			} else if (key == "b") { // The signature
				this->m_Signature = val;
			} else if (key == "h") {
				parseHeaders(val);
			} else if (key == "x") {
				try { this->m_ExpireDate = stol(val); }
				catch (...) { this->m_ExpireDate = 0; }
			} else if (key == "t") {
				try { this->m_SignDate = stol(val); }
				catch (...) { this->m_SignDate = 0; }
			}
		});

		return *this;
	}


	const char *DKIMHeader::getVersionString()
	{ return __dkimHeaderVersionToString(this->m_Version); }

	const char *DKIMHeader::getCanonAlgPairString()
	{ return __dkimHeaderCanonAlgPairToString(this->m_CanonAlgoPair); }

	const char *DKIMHeader::getHeaderAlgString()
	{ return __dkimHeaderAlgToString(this->m_HeaderAlgorithm); }

	string DKIMHeader::getHeaderString() const {
		string result;

		for_each(this->m_Headers.begin(), this->m_Headers.end(), [&](const string &h) {
			result += h + ':';
		});

		result.pop_back();
		return result;
	}

	DKIMHeaderCanonAlgPair DKIMHeader::getCanonAlgorithmPair()
	{ return this->m_CanonAlgoPair; }

	DKIMHeaderAlgorithm DKIMHeader::getHeaderAlgorithm()
	{ return this->m_HeaderAlgorithm; }

	const string &DKIMHeader::getBodyHash()
	{ return this->m_BodyHash; }

	const string &DKIMHeader::getKeySelector()
	{ return this->m_KeySelector; }
	
	const string &DKIMHeader::getDomain()
	{ return this->m_Domain; }

	const string &DKIMHeader::getSignature()
	{ return this->m_Signature; }

	const vector<string> &DKIMHeader::getHeaders()
	{ return this->m_Headers; }

	DKIMHeader &DKIMHeader::setBodyHash(const string &bodyHash)
	{ this->m_BodyHash = bodyHash; return *this; }

	DKIMHeader &DKIMHeader::setSignature(const string &signature)
	{ this->m_Signature = signature; return *this; }

    DKIMHeader &DKIMHeader::setDomain(const string &domain)
    { this->m_Domain = domain; return *this; }
    
    DKIMHeader &DKIMHeader::setKeySelector(const string &keySelector)
    { this->m_KeySelector = keySelector; return *this; }

    DKIMHeader &DKIMHeader::setHeaders(const vector<string> &headers)
    { this->m_Headers = headers; return *this; }

    DKIMHeader &DKIMHeader::setCanonAlgoPair(const DKIMHeaderCanonAlgPair &canonAlgo)
    { this->m_CanonAlgoPair = canonAlgo; return *this; }

    DKIMHeader &DKIMHeader::setHeaderAlgo(const DKIMHeaderAlgorithm &algo)
    { this->m_HeaderAlgorithm = algo; return *this; }

	DKIMHeader &DKIMHeader::setExpireDate(int64_t t)
	{ this->m_ExpireDate = t; return *this; }
    
	DKIMHeader &DKIMHeader::setSignDate(int64_t t)
	{ this->m_SignDate = t; return *this; }

    string DKIMHeader::build() const {
		vector<pair<string, string>> values = {
    		make_pair("v", __dkimHeaderVersionToString(this->m_Version)),
    		make_pair("s", this->m_KeySelector),
			make_pair("h", this->getHeaderString()),
    		make_pair("d", this->m_Domain),
    		make_pair("c", __dkimHeaderCanonAlgPairToString(this->m_CanonAlgoPair)),
    		make_pair("a", __dkimHeaderAlgToString(this->m_HeaderAlgorithm))
		};

		if (this->m_ExpireDate != 0 && this->m_SignDate != 0) {
			values.push_back(make_pair("t", to_string(this->m_SignDate)));
			values.push_back(make_pair("x", to_string(this->m_ExpireDate)));
		}

		values.push_back(make_pair("bh", this->m_BodyHash));
		values.push_back(make_pair("b", this->m_Signature));

    	return Builders::buildHeaderFromSegments("DKIM-Signature", values);
    }

	DKIMHeader &DKIMHeader::print(Logger &logger) {
		logger << DEBUG;

		logger << "DKIMHeader {";
		logger << "\tVersion: " << this->getVersionString() << ENDL;
		logger << "\tCanonicalization algo pair: " << this->getCanonAlgPairString() << ENDL;
		logger << "\tHeader algorithm: " << this->getHeaderAlgString() << ENDL;
		logger << "\tDomain: " << this->m_Domain << ENDL;
		logger << "\tKey selector: " << this->m_KeySelector << ENDL;
		logger << "\tBody hash: " << this->m_BodyHash << ENDL;
		logger << "\tSignature: " << this->m_Signature << ENDL;
		logger << "\tSign date: " << this->m_SignDate << ENDL;
		logger << "\tExpire date: " << this->m_ExpireDate << ENDL;
		logger << ENDL;
		logger << "\tHeaders: " << ENDL;
		size_t i = 0;
		for_each(this->m_Headers.begin(), this->m_Headers.end(), [&](const string &h) {
			logger << "\t\t" << i++ << " -> '" << h << '\'' << ENDL;
		});
		logger << "}" << ENDL;

		logger << CLASSIC;
		return *this;
	}

	DKIMHeader::~DKIMHeader() = default;
}