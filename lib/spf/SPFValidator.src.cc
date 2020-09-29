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

#include "SPFValidator.src.h"

namespace FSMTP::SPF {
  SPFValidator::SPFValidator():
    m_Logger("SPFValidator", LoggerLevel::DEBUG)
  {
    this->m_Result.type = SPFValidatorResultType::ResultTypeDenied;
    this->m_Result.details = "Sender not authorized";
  }

  bool SPFValidator::validate(const string &query, const string &cmp) {
    auto &logger = this->m_Logger;
    SPFRecord record = SPFRecord::fromDNS(query.c_str());
    DEBUG_ONLY(record.print(logger));

    // Checks if we need to redirect
    if (!record.getRedirect().empty()) {
      DEBUG_ONLY(logger << "Well, redirecting to: '" << record.getRedirect() << '\'' << ENDL);
      return this->validate(record.getRedirect(), cmp);
    }

    // ========================================
    // Searches for the address in records
    // ========================================
    
    DNS::Resolver resolver;

    auto validateMX = [&](const string &domain) {
      vector<DNS::RR> records = resolver.query(domain.c_str(), ns_t_mx).initParse().getRecords();
      for (const DNS::RR &rr : records) {
        if (Networking::isAddress(rr.getData(), Networking::AddrType::AT_IPv4)) {
          DEBUG_ONLY(logger << "MX record '" << rr.getData() << "' is already an address, comparing .." << ENDL);
          if (Networking::addr_compare(cmp, rr.getData(), Networking::AddrType::AT_IPv4)) {
            this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
            this->m_Result.details = cmp + " is listed in MX records of " + query;
            return true;
          } else continue;
        }

        string address = DNS::resolveHostname(rr.getData());
        DEBUG_ONLY(logger << "Resolved MX record '" << rr.getData() << "' to: '" << address << '\'' << ENDL);
        if (Networking::addr_compare(cmp, address, Networking::AddrType::AT_IPv4)) {
          this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
          this->m_Result.details = cmp + " is listed in MX records of " + query;
          return true;
        }
      }

      return false;
    };

    auto validateA = [&](const string &a) {
      vector<string> addresses = DNS::resolveAllFromHostname(query);
      for (const string &addr : addresses) {
        DEBUG_ONLY(logger << "Resolved A record '" << addr << '\'' << ENDL);
        if (Networking::addr_compare(cmp, addr, Networking::AddrType::AT_IPv4)) {
          this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
          this->m_Result.details = cmp + " is listed in A records of " + query;
          return true;
        }
      }

      return false;
    };

    // If MX records allowed, check the MX records for matches
    if (record.getMXRecordsAllowed()) {
      try {
        if (validateMX(query)) return true;
      } catch (...) {}
    }

    // If a records allowed, check the a records for matches
    if (record.getARecordsAllowed()) {
      try {
        if (validateA(query)) return true;
      } catch (...) {}
    }

    // Loops over the IPv4 addresses and compares them against the client's one
    if (record.getIPv4s().size() > 0) {
      for (const string &addr : record.getIPv4s()) {
        if (Networking::addr_compare(cmp, addr, Networking::AddrType::AT_IPv4)) {
          this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
          this->m_Result.details = cmp + " is listed in IPv4 addresses of " + query;
          return true;
        }
      }
    }

    // Loops over the MX records from the domains specified in the
    //  SPF Record
    if (record.getMXRecords().size() > 0) {
      for (const string &domain : record.getMXRecords()) {
        try {
          if (validateMX(domain)) return true;
        } catch (...) {}
      }
    }

    // Loops over the a records from the domains specified by the
    //  SPF record
    if (record.getARecords().size() > 0) {
      for (const string &domain : record.getARecords()) {
        try {
          if (validateA(domain)) return true;
        } catch (...) {}
      }
    }

    // Loops over the specified PTR records, and compars it against the
    //  reverse lookup of the current IP address
    if (record.getPTRRecords().size() > 0) {
      // Creates the sockaddr_in version of the input address
      //  so we can perform the reverse lookup
      struct sockaddr_in cmpAddr;
      memset(&cmpAddr, 0, sizeof (cmpAddr));
      cmpAddr.sin_addr.s_addr = inet_addr(cmp.c_str());
      cmpAddr.sin_family = AF_INET;

      // Performs the reverse lookup
      string reverse = DNS::getHostnameByAddress(&cmpAddr);
      DEBUG_ONLY(logger << "Reverse lookup of address '" << cmp << "' is '" << reverse << '\'' << ENDL);

      for (const string &record : record.getPTRRecords()) {
        if (reverse.length() < record.length()) continue;
        if (reverse.substr(reverse.length() - record.length()) == record) {
          this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
          this->m_Result.details = "Reverse lookup of " + cmp + " matches PTR records for " + query;
          return true;
        }
      }
    }

    // ========================================
    // Inherits SPF from netblocks / domains
    // ========================================

    if (record.getDomains().size() > 0) {
      for (const string &domain : record.getDomains()) {
        try {
          if (this->validate(domain, cmp)) return true;
        } catch (...) {}
      }
    }

    return false;
  }

  bool SPFValidator::safeValidate(const string &query, const string &cmp) {
    try {
      return this->validate(query, cmp);
    } catch (...) {
      this->m_Result.type = SPFValidatorResultType::ResultTypeSystemFailure;
      this->m_Result.details = "Failed to validate SPF";
      return false;
    }
  }

  const SPFValidatorResult &SPFValidator::getResult() { return this->m_Result; }
  string SPFValidator::getResultString() {
    string result;

    switch (this->m_Result.type) {
      case SPFValidatorResultType::ResultTypeAllowed: result += "pass "; break;
      case SPFValidatorResultType::ResultTypeDenied: result += "fail "; break;
      case SPFValidatorResultType::ResultTypeSystemFailure: result += "neutral "; break;
    }

    result += '(' + this->m_Result.details + ')';

    return result;
  }

  SPFValidator::~SPFValidator() = default;
}