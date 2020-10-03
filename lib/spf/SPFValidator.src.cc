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
    DNS::Resolver resolver;
    DEBUG_ONLY(record.print(logger));

    // Checks if we need to redirect
    if (!record.getRedirect().empty()) {
      DEBUG_ONLY(logger << "Well, redirecting to: '" << record.getRedirect() << '\'' << ENDL);
      return this->validate(record.getRedirect(), cmp);
    }

    // ========================================
    // Parses the IPvN address into struct
    // ========================================

    struct in_addr ipv4;
    struct in6_addr ipv6;
    const void *ipvn = nullptr;

    switch (this->m_Protocol) {
      case Networking::IP::Protocol::Protocol_IPv4:
        if (inet_pton(AF_INET, cmp.c_str(), &ipv4) != 1) {
          logger << ERROR << "Invalid IPv4 address" << ENDL << CLASSIC;
          return false;
        }
        
        ipvn = &ipv4;
        break;
      case Networking::IP::Protocol::Protocol_IPv6:
        if (inet_pton(AF_INET6, cmp.c_str(), &ipv6) != 1) {
          logger << ERROR << "Invalid IPv6 address" << ENDL << CLASSIC;
          return false;
        }

        ipvn = &ipv6;
        break;
      default: {
        logger << ERROR << "No valid protocol specified" << ENDL << CLASSIC;
        return false;
      }
    }

    // ========================================
    // Validates the IPvN addresses
    // ========================================

    // Checks the protocol and performs the validation
    //  for the addresses listed in the records
    switch (this->m_Protocol) {
      case Networking::IP::Protocol::Protocol_IPv4:
        if (this->validateIPv4(query, ipv4, record.getIPv4s())) return true;
        break;
      case Networking::IP::Protocol::Protocol_IPv6:
        if (this->validateIPv6(query, ipv6, record.getIPv6s())) return true;
        break;
      default: break;
    }

    // ========================================
    // Performs the checks
    // ========================================
    
    if (record.getMXRecordsAllowed()) {
      try {
        if (this->validateMX(resolver, ipvn, query)) return true;
      } catch (...) {}
    }

    if (record.getARecordsAllowed()) {
      try {
        if (this->validateA(resolver, ipvn, query)) return true;
      } catch (...) {}
    }

    if (record.getMXRecords().size() > 0) {
      for (const string &domain : record.getMXRecords()) {
        try {
          if (this->validateMX(resolver, ipvn, query)) return true;
        } catch (...) {}
      }
    }

    if (record.getARecords().size() > 0) {
      for (const string &domain : record.getARecords()) {
        try {
          if (this->validateA(resolver, ipvn, domain)) return true;
        } catch (...) {}
      }
    }

    if (record.getPTRRecords().size() > 0) {
      try {
        if (this->validatePTR(ipvn, record.getPTRRecords())) return true;
      } catch (...) {}
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

  bool SPFValidator::safeValidate(const string &query, string cmp) {
    try {
      return this->validate(query, cmp);
    } catch (...) {
      this->m_Result.type = SPFValidatorResultType::ResultTypeSystemFailure;
      this->m_Result.details = "Failed to validate SPF";
      return false;
    }
  }

  bool SPFValidator::validateIPv6(const string &query, const struct in6_addr &addr, const vector<string> &compareAddresses) {
    for (const string &compareAddress : compareAddresses) {
      try {
        if (Networking::IPv6::compare(addr, compareAddress)) {
          this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;

          // Checks if the compare address is an subnet, if so use subnet
          //  in the message, and set the details
          if (compareAddress.find_first_of('/') != string::npos)
            this->m_Result.details = "part of IPv6 subnet [" + compareAddress + "] for " + query;
          else this->m_Result.details = "IPv6 [" + compareAddress + "] listed in " + query;
          
          return true;
        }
      } catch (...) {}
    }

    return false;
  }

  bool SPFValidator::validateMX(DNS::Resolver &resolver, const void *cmp, const string &domain) {
    auto &logger = this->m_Logger;
    
    vector<DNS::RR> records = resolver.query(domain.c_str(), ns_t_mx).initParse().getRecords();
    for (const DNS::RR &rr : records) {
      struct in_addr ipv4;
      struct in6_addr ipv6;

      // Checks if the current record is IPv4 or IPv6, else we see
      //  it as it is pointing to andifferent record, and then will 
      //  resolve that
      if (inet_pton(AF_INET, rr.getData().c_str(), &ipv4) == 1) {
        if (this->m_Protocol != Networking::IP::Protocol::Protocol_IPv4) continue;
        
        if (Networking::IPv4::compare(ipv4, *reinterpret_cast<const struct in_addr *>(cmp))) {
          this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
          this->m_Result.details = "IPv4 [" + rr.getData() + "] in MX of " + domain;
          return true;
        }
      } else if (inet_pton(AF_INET6, rr.getData().c_str(), &ipv6) == 1) {
        if (this->m_Protocol != Networking::IP::Protocol::Protocol_IPv6) continue;

        if (Networking::IPv6::compare(ipv6, *reinterpret_cast<const struct in6_addr *>(cmp))) {
          this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
          this->m_Result.details = "IPv6 [" + rr.getData() + "] in MX of " + domain;
          return true;
        }
      } else {
        string resolvedAddress = DNS::resolveHostname(rr.getData());
        if (this->compare(cmp, resolvedAddress)) {
          this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
          this->m_Result.details = "IPvN [" + resolvedAddress + "] in resolved MX of " + domain;
          return true;
        }
      }
    }

    return false;
  }

  bool SPFValidator::validateA(DNS::Resolver &resolver, const void *cmp, const string &query) {
    auto &logger = this->m_Logger;
    
    // Checks which protocol we're using, which we base the check uppon
    //  since it can be either A or AAAA
    switch (this->m_Protocol) {
      case Networking::IP::Protocol::Protocol_IPv4: {
        for (const string &addr : DNS::resolveAllFromHostname(query, AF_INET)) {
          DEBUG_ONLY(logger << "Resolved A record '" << addr << '\'' << ENDL);
      
          if (this->compare(cmp, addr)) {
            this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
            this->m_Result.details = "IPv4 [" + addr + "] in A of " + query;
            return true;
          }
        }
        break;
      }
      case Networking::IP::Protocol::Protocol_IPv6: {
        for (const string &addr : DNS::resolveAllFromHostname(query, AF_INET6)) {
          DEBUG_ONLY(logger << "Resolved AAAA record '" << addr << '\'' << ENDL);
          
          if (this->compare(cmp, addr)) {
            this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
            this->m_Result.details = "IPv6 [" + addr + "] in AAAA of " + query;
            return true;
          }
        }
        break;
      }
    }
  
    return false;
  }

  bool SPFValidator::validateIPv4(const string &query, const struct in_addr &addr, const vector<string> &compareAddresses) {
    for (const string &compareAddress : compareAddresses) {
      try {
        if (Networking::IPv4::compare(addr, compareAddress)) {
          this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;

          // Checks if the compare address is an subnet, if so use subnet
          //  in the message, and set the details
          if (compareAddress.find_first_of('/') != string::npos)
            this->m_Result.details = "part of IPv4 subnet [" + compareAddress + "] for " + query;
          else this->m_Result.details = "IPv4 [" + compareAddress + "] listed in " + query;
          return true;
        }
      } catch (...) {}
    }

    return false;
  }

  bool SPFValidator::validatePTR(const void *cmp, const vector<string> &names) {
    auto &logger = this->m_Logger;
    string reverse;
    
    // Checks the protocol and performs the reverse lookup
    //  this will be used to check the domain extension
    switch (this->m_Protocol) {
      case Networking::IP::Protocol::Protocol_IPv4: {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof (struct sockaddr_in));
        addr.sin_addr = *reinterpret_cast<const in_addr *>(cmp);
        addr.sin_family = AF_INET;

        reverse = DNS::getHostnameByAddress<struct sockaddr_in>(&addr);
        break;
      }
      case Networking::IP::Protocol::Protocol_IPv6: {
        struct sockaddr_in6 addr;
        memset(&addr, 0, sizeof (struct sockaddr_in6));
        addr.sin6_addr = *reinterpret_cast<const in6_addr *>(cmp);
        addr.sin6_family = AF_INET6;

        reverse = DNS::getHostnameByAddress<struct sockaddr_in6>(&addr);
        break;
      }
      default: return false;
    }

    // Prints the reverse result
    DEBUG_ONLY(logger << DEBUG << "Got PTR reverse result: '" << reverse << '\'' << ENDL << CLASSIC);

    // Checks the reverse result, and if the extension matches
    //  one in the names vector
    for (const string &name : names) {
      if (reverse.length() < name.length()) continue;
      if (reverse.substr(reverse.length() - name.length()) == name) {
        this->m_Result.type = SPFValidatorResultType::ResultTypeAllowed;
        this->m_Result.details = "Reverse lookup " + reverse + " has extension " + name;
        return true;
      }
    }

    return false;
  }

  bool SPFValidator::compare(const void *addr, const string &cmp) {
    switch (this->m_Protocol) {
      case Networking::IP::Protocol::Protocol_IPv4:
        return Networking::IPv4::compare(*reinterpret_cast<const struct in_addr *>(addr), cmp);
      case Networking::IP::Protocol::Protocol_IPv6:
        return Networking::IPv6::compare(*reinterpret_cast<const struct in6_addr *>(addr), cmp);
      default: return false;
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

  SPFValidator &SPFValidator::setProtocol(Networking::IP::Protocol p) {
    this->m_Protocol = p;
  }

  SPFValidator::~SPFValidator() = default;
}