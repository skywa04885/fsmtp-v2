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

#include "DKIMRecord.src.h"

namespace FSMTP::DKIM {
  const char *__dkimRecordVersionToString(DKIMRecordVersion v) {
    switch (v) {
      case DKIMRecordVersion::RecordVersionDKIM1: return "DKIM1";
    }
  }

  const char *__dkimRecordAlgorithmToString(DKIMRecordAlgorithm a) {
    switch (a) {
      case DKIMRecordAlgorithm::RecordAlgorithmRSA: return "RSA";
    }
  }

  string __dkimRecordAllowedHashingAlgosToString(int32_t flags) {
    string result;

    if ((flags & _FSMTP_DKIM_RECORD_FLAG_ALLOWED_HASH_ALGO_SHA256) == _FSMTP_DKIM_RECORD_FLAG_ALLOWED_HASH_ALGO_SHA256)
      result += "SHA256/";
    if ((flags & _FSMTP_DKIM_RECORD_FLAG_ALLOWED_HASH_ALGO_SHA1) == _FSMTP_DKIM_RECORD_FLAG_ALLOWED_HASH_ALGO_SHA1)
      result += "SHA1/";

    if (result.size() > 0) result.pop_back();
    return result;
  }

  string __dkiMRecordAllowedServicesToString(int32_t flags) {
    string result;

    if ((flags & _FSMTP_DKIM_RECORD_FLAG_ALLOWED_SERVICE_EMAIL) == _FSMTP_DKIM_RECORD_FLAG_ALLOWED_SERVICE_EMAIL)
      result += "Email/";

    if (result.size() > 0) result.pop_back();
    return result;
  }

  DKIMRecord::DKIMRecord():
    m_Flags(0x0), m_Algorithm(DKIMRecordAlgorithm::RecordAlgorithmRSA),
    m_Version(DKIMRecordVersion::RecordVersionDKIM1)
  {}

  DKIMRecord &DKIMRecord::parse(const string &raw) {
    // ==================================
		// Splits the record into segments
		// ==================================

    size_t start = 0, end = raw.find_first_of(';');
    vector<string> segments = {};

    for (;;) {
      segments.push_back(raw.substr(start, end - start));
      if (end == string::npos) break;

      start = end + 1;
      end = raw.find_first_of(';', start);
    }

    // ==================================
		// Makes sense of the segments
		// ==================================

    for_each(segments.begin(), segments.end(), [&](const string &seg) {
      if (seg.empty()) return;

      // Splits the segment into a key / value pair, so we can
      //  make sense of the data in it
      size_t sep = string::npos;
      if ((sep = seg.find_first_of('=')) == string::npos)
        throw runtime_error(EXCEPT_DEBUG("Could not parse k/v pair from: '" + seg + '\''));
      
      string key = seg.substr(0, sep), val = seg.substr(++sep);
      transform(key.begin(), key.end(), key.begin(), [](const char c) { return tolower(c); });
			if (*key.begin() == ' ') key.erase(key.begin(), key.begin() + 1);
			if (*(key.end() - 1) == ' ') key.pop_back();
			
			transform(val.begin(), val.end(), val.begin(), [](const char c) { return tolower(c); });
			if (*val.begin() == ' ') val.erase(val.begin(), val.begin() + 1);
			if (*(val.end() - 1) == ' ') val.pop_back();

      auto parseHashAlgorithms = [&](const string &val) {
        start = 0, end = val.find_first_of(':');

        for (;;) {
          string seg = val.substr(start, end - start);
          transform(seg.begin(), seg.end(), seg.begin(), [](const char c) { return tolower(c); });
          if (*seg.begin() == ' ') seg.erase(seg.begin(), seg.begin() + 1);
          if (*(seg.end() - 1) == ' ') seg.pop_back();

          if (seg == "sha1") this->m_Flags |= _FSMTP_DKIM_RECORD_FLAG_ALLOWED_HASH_ALGO_SHA1;
          else if (seg == "sha256") this->m_Flags |= _FSMTP_DKIM_RECORD_FLAG_ALLOWED_HASH_ALGO_SHA256;
          
          if (end == string::npos) break;
          start = end + 1;
          end = val.find_first_of(':', start);
        }
      };

      auto parseServices = [&](const string &val) {
        start = 0, end = val.find_first_of(':');

        for (;;) {
          string seg = val.substr(start, end - start);
          transform(seg.begin(), seg.end(), seg.begin(), [](const char c) { return tolower(c); });
          if (*seg.begin() == ' ') seg.erase(seg.begin(), seg.begin() + 1);
          if (*(seg.end() - 1) == ' ') seg.pop_back();

          if (seg == "email") this->m_Flags |= _FSMTP_DKIM_RECORD_FLAG_ALLOWED_SERVICE_EMAIL;
          
          if (end == string::npos) break;
          start = end + 1;
          end = val.find_first_of(':', start);
        }
      };

      auto parseTags = [&](const string &val) {
        start = 0, end = val.find_first_of(':');

        for (;;) {
          string seg = val.substr(start, end - start);
          transform(seg.begin(), seg.end(), seg.begin(), [](const char c) { return tolower(c); });
          if (*seg.begin() == ' ') seg.erase(seg.begin(), seg.begin() + 1);
          if (*(seg.end() - 1) == ' ') seg.pop_back();

          if (seg == "y") this->m_Flags |= _FSMTP_DKIM_RECORD_FLAG_TESTING;
          else if (seg == "s") this->m_Flags |= _FSMTP_DKIM_RECORD_FLAG_SAME_DOMAIN;

          if (end == string::npos) break;
          start = end + 1;
          end = val.find_first_of(':', start);
        }
      };

      // Checks the key and then stores the value inside of the current
      //  class instance, so we can read it later
      if (key == "v") { // The dkim version
        if (val == "dkim1") this->m_Version = DKIMRecordVersion::RecordVersionDKIM1;
        else this->m_Version = DKIMRecordVersion::RecordVersionDKIM1;
      } else if (key == "p") { // The public key
        this->m_PublicKey = val;
      } else if (key == "k") { // The key algorithm
        if (val == "rsa") this->m_Algorithm = DKIMRecordAlgorithm::RecordAlgorithmRSA;
        else this->m_Algorithm = DKIMRecordAlgorithm::RecordAlgorithmRSA;
      } else if (key == "h") { // The allowed hashing algorithms
        parseHashAlgorithms(val);
      } else if (key == "s") { // The allowed services
        parseServices(val);
      } else if (key == "t") { // The tags
        parseTags(val);
      }
    });

    return *this;
  }

  const char *DKIMRecord::getVersionString() {
    return __dkimRecordVersionToString(this->m_Version);  
  }
  
  const char *DKIMRecord::getAlgorithmString() {
    return __dkimRecordAlgorithmToString(this->m_Algorithm);
  }

  string DKIMRecord::getAllowedHashAlgosString() {
    return __dkimRecordAllowedHashingAlgosToString(this->m_Flags);
  }
  
  string DKIMRecord::getAllowedServicesString() {
    return __dkiMRecordAllowedServicesToString(this->m_Flags);
  }

  const string &DKIMRecord::getPublicKey() {
    return this->m_PublicKey;
  }

  DKIMRecord &DKIMRecord::print(Logger &logger) {
    logger << DEBUG;

    logger << "DKIMRecord {" << ENDL;
    logger << "\tVersion: " << this->getVersionString() << ENDL;
    logger << "\tAlgorithm: " << this->getAlgorithmString() << ENDL;
    logger << "\tBinary flags: " << bitset<32>(this->m_Flags) << ENDL;
    logger << "\tAllowed hashes: " << this->getAllowedHashAlgosString() << ENDL;
    logger << "\tAllowed services: " << this->getAllowedServicesString() << ENDL;
    logger << "\tPublic key: " << this->m_PublicKey << ENDL;
    logger << "}";

    logger << CLASSIC;
  }

  DKIMRecord DKIMRecord::fromDNS(const char *query) {
    DKIMRecord result;
    DNS::Resolver resolver;
    vector<DNS::RR> records = resolver.query(query, ns_t_txt).initParse().getTXTRecords();

    // Loops over the records, and attempts to find an record which contains
    //  dkim data
    bool found = false;
    any_of(records.begin(), records.end(), [&](const DNS::RR &rr) {
      if (rr.getData().length() < 6) return true;

        cout << rr.getData() << endl;

      // Checks if it is an DKIM record
      string cmp = rr.getData().substr(rr.getData().find_first_of('v'), 6);
      transform(cmp.begin(), cmp.end(), cmp.begin(), [](const char c) { return tolower(c); });
      if (cmp != "v=dkim") return true;

      // Attempts to parse the dkim record
      try {
        result.parse(rr.getData());
        found = true;
        return false;
      } catch (...) {}

      return true;
    });

    return result;
  }

  DKIMRecord::~DKIMRecord() = default;
}
