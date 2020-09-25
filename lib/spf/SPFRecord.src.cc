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

#include "SPFRecord.src.h"

namespace FSMTP::SPF {
  string __spfFlagsToString(int32_t f) {
    string result;

    if ((f & _FSMTP_SPF_RECORD_FLAG_ALLOW_MX) == _FSMTP_SPF_RECORD_FLAG_ALLOW_MX)
      result += "MX/";
    if ((f & _FSMTP_SPF_RECORD_FLAG_ALLOW_A) == _FSMTP_SPF_RECORD_FLAG_ALLOW_A)
      result += "A/";
    
    if (result.length() > 0) result.pop_back();
    return result;
  }

  const char *__spfVersionToString(SPFVersion v) {
    switch (v) {
      case SPFVersion::SPF1: return "SPF1";
    } 
  }

  const char *__spfPolicyToString(SPFPolicy p) {
    switch (p) {
      case SPFPolicy::PolicyReject: return "reject";
      case SPFPolicy::PolicyJunk: return "junk";
      case SPFPolicy::PolicyAllowAll: return "allow all";
      case SPFPolicy::PolicyNoValidation: return "no validation";
    }
  }

  SPFRecord::SPFRecord():
    m_Flags(0x0), m_Policy(SPFPolicy::PolicyAllowAll), m_Version(SPFVersion::SPF1)
  {}

  SPFRecord &SPFRecord::parse(const string &raw) {
    #ifdef _SMTP_DEBUG
    Logger logger("SPFRecord::parse", LoggerLevel::DEBUG);
    logger << "Parsing SPF Record: '" << raw << '\'' << ENDL;
    #endif

    // ================================
    // Splits the header into segments
    // ================================

    vector<string> segments = {};
    size_t start = 0, end = raw.find_first_of(' ');
    for (;;) {
      segments.push_back(raw.substr(start, end - start));
      if (end == string::npos) break;

      start = end + 1;
      end = raw.find_first_of(' ', start);
    }

    // ================================
    // Makes sense of the header
    // ================================

    for_each(segments.begin(), segments.end(), [&](const string &seg) {
      if (seg.empty()) return;

      // Checks if the header is a key value pair, and not just a
      //  single atom
      if (seg.find_first_of(':') != string::npos || seg.find_first_of('=') != string::npos) {
        // Parses the key / value pair from the header, so we then
        //  can check how to use the value
        size_t colonSep = seg.find_first_of(':'), eqSep = seg.find_first_of('=');
        size_t sep = string::npos;
        if ((sep = colonSep == string::npos ? eqSep : colonSep) == string::npos)
          throw runtime_error(EXCEPT_DEBUG("Could not parse k/v pair from: '" + seg + '\''));

        string key = seg.substr(0, sep), val = seg.substr(++sep);
        transform(key.begin(), key.end(), key.begin(), [](const char c) { return tolower(c); });
        if (*key.begin() == ' ') key.erase(key.begin(), key.begin() + 1);
        if (*(key.end() - 1) == ' ') key.pop_back();

        transform(val.begin(), val.end(), val.begin(), [](const char c) { return tolower(c); });
        if (*val.begin() == ' ') val.erase(val.begin(), val.begin() + 1);
        if (*(val.end() - 1) == ' ') val.pop_back();

        // Checks the key, and if we can actually do something useful with it
        //  if not, just ignore it
        if (key == "mx" || key == "+mx") { // Allow MX of specified domain
          this->m_AllowedMXs.push_back(val);
        } else if (key == "ptr" || key == "+ptr") { // Allow PTR of specified domain
          this->m_AllowedPTRs.push_back(val);
        } else if (key == "a" || key == "+a") { // Allow A of specified domain
          this->m_AllowedAs.push_back(val);
        } else if (key == "include" || key == "+include") { // Inherit SPF from specified domain
          this->m_AllowedDomains.push_back(val);
        } else if (key == "ip4" || key == "+ip4") { // Allow specified IPv4 Address
          this->m_AllowedIPv4s.push_back(val);
        } else if (key == "ip6" || key == "+ip6") { // Allow specified IPv6 address
          this->m_AllowedIPv6s.push_back(val);
        } else if (key == "redirect") { // Redirect to different subdomain or domain
          this->m_Redirect = val;
        }
      } else { // Is a single atom
        string atom = seg;
        transform(atom.begin(), atom.end(), atom.begin(), [](const char c) { return tolower(c); });
        if (*atom.begin() == ' ') atom.erase(atom.begin(), atom.begin() + 1);
        if (*(atom.end() - 1) == ' ') atom.pop_back();

        // Checks the atom, and if we can make sense of it
        //  else just ignore it
        if (atom == "mx" || atom == "+mx") { // If MX records from the domain are allowed
          this->m_Flags |= _FSMTP_SPF_RECORD_FLAG_ALLOW_MX;
        } else if (atom == "a" || atom == "+a") { // If A records from the domain are allowed
          this->m_Flags |= _FSMTP_SPF_RECORD_FLAG_ALLOW_A;
        } else if (atom == "~all") { // Put email in spam if not authorized by SPF
          this->m_Policy = SPFPolicy::PolicyJunk;
        } else if (atom == "-all") { // Reject an email if not in SPF
          this->m_Policy = SPFPolicy::PolicyReject;
        } else if (atom == "?all") { // Unauthorized servers allowed
          this->m_Policy = SPFPolicy::PolicyNoValidation;
        } else if (atom == "+all") { // All servers are allowed
          this->m_Policy = SPFPolicy::PolicyAllowAll;
        }
      }
    });

    return *this;
  }

  const char *SPFRecord::getVersionString() {
    return __spfVersionToString(this->m_Version);
  }

  string SPFRecord::getFlagsString() {
    return __spfFlagsToString(this->m_Flags);
  }

  const char *SPFRecord::getPolicyString() {
    return __spfPolicyToString(this->m_Policy);
  }

  const vector<string> &SPFRecord::getIPv4s() { return this->m_AllowedIPv4s; }
  const vector<string> &SPFRecord::getIPv6s() { return this->m_AllowedIPv6s; }
  const vector<string> &SPFRecord::getMXRecords() { return this->m_AllowedMXs; }
  const vector<string> &SPFRecord::getARecords() { return this->m_AllowedAs; }
  const vector<string> &SPFRecord::getDomains() { return this->m_AllowedDomains; }
  const vector<string> &SPFRecord::getPTRRecords() { return this->m_AllowedPTRs; }
  const string &SPFRecord::getRedirect() { return this->m_Redirect; }
  SPFPolicy SPFRecord::getPolicy() { return this->m_Policy; }
  SPFVersion SPFRecord::getVersion() { return this->m_Version; }

  bool SPFRecord::getMXRecordsAllowed() {
    return ((this->m_Flags & _FSMTP_SPF_RECORD_FLAG_ALLOW_MX) == _FSMTP_SPF_RECORD_FLAG_ALLOW_MX);
  }

  bool SPFRecord::getARecordsAllowed() {
    return ((this->m_Flags & _FSMTP_SPF_RECORD_FLAG_ALLOW_A) == _FSMTP_SPF_RECORD_FLAG_ALLOW_A);
  }

  SPFRecord &SPFRecord::print(Logger &logger) {
    logger << DEBUG;
    logger << "SPFRecord {" << ENDL;

    logger << "\tVersion: " << this->getVersionString() << ENDL;
    logger << "\tBinary Flags: " << bitset<32>(this->m_Flags) << ENDL;
    logger << "\tReadable flags: " << this->getFlagsString() << ENDL;
    logger << "\tPolicy: " << this->getPolicyString() << ENDL;
    logger << "\tRedirect: " << (this->m_Redirect.empty() ? "No" : "Yes, to: '" + this->m_Redirect + '\'') << ENDL;
    logger << ENDL;

    auto printList = [&](const vector<string> &l) {
      size_t i = 0;
      for_each(l.begin(), l.end(), [&](const string &s) {
        logger << "\t\t" << i++ << " -> '" << s << '\'' << ENDL;
      });
    };

    logger << "\tAllowed IPv4's:" << ENDL;
    printList(this->m_AllowedIPv4s);

    logger << "\tAllowed IPv6's:" << ENDL;
    printList(this->m_AllowedIPv6s);

    logger << "\tAllowed PTR records:" << ENDL;
    printList(this->m_AllowedPTRs);

    logger << "\tAllowed MX records:" << ENDL;
    printList(this->m_AllowedMXs);

    logger << "\tAllowed A records:" << ENDL;
    printList(this->m_AllowedAs);

    logger << "\tInherit from domains:" << ENDL;
    printList(this->m_AllowedDomains);

    logger << "}" << ENDL;
    logger << CLASSIC;

    return *this;
  }

  SPFRecord SPFRecord::fromDNS(const char *query) {
    SPFRecord result;

    // Gets the vector of records from the query
    DNS::Resolver resolver;
    vector<DNS::RR> records = resolver.query(query, ns_t_txt).initParse().getTXTRecords();
    
    // Loops over the records and checks if one of them is an valid SPF record
    bool found = false;
    all_of(records.begin(), records.end(), [&](const DNS::RR &rr) {
      if (rr.getData().length() <= 5) return true;
      else if (rr.getData().find_first_of('v') == string::npos) return true;

      // Checks if it actually is an spf record
      string cmp = rr.getData().substr(rr.getData().find_first_of('v'), 5);
      transform(cmp.begin(), cmp.end(), cmp.begin(), [](const char c) { return tolower(c); });
      if (cmp != "v=spf") return true;

      // Attempts to parse the SPF record
      try {
        result.parse(rr.getData());
        found = true;
        return false;
      }
      catch (const runtime_error &e) {}
    });

    // Checks if an SPF record was found, else just
    //  throw an runtime error
    if (!found) throw runtime_error(EXCEPT_DEBUG("Could not find valid SPF record"));
    return result;
  }

  SPFRecord::~SPFRecord() = default;
}