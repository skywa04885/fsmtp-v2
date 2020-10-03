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

#ifndef _LIB_SPF_VALIDATOR_H
#define _LIB_SPF_VALIDATOR_H

#include "../default.h"
#include "SPFRecord.src.h"
#include "../general/Logger.src.h"
#include "../networking/IP.src.h"
#include "../networking/IPv4.src.h"
#include "../networking/IPv6.src.h"

namespace FSMTP::SPF {
  enum SPFValidatorResultType {
    ResultTypeDenied, ResultTypeAllowed,
    ResultTypeSystemFailure
  };

  struct SPFValidatorResult {
    SPFValidatorResultType type;
    string details;
  };

  class SPFValidator {
  public:
    SPFValidator();

    bool validate(const string &query, const string &cmp);
    bool safeValidate(const string &query, string cmp);
    
    bool validateIPv6(const string &query, const struct in6_addr &addr, const vector<string> &compareAddresses);
    bool validateIPv4(const string &query, const struct in_addr &addr, const vector<string> &compareAddresses);
    bool validateMX(DNS::Resolver &resolver, const void *cmp, const string &domain);
    bool validateA(DNS::Resolver &resolver, const void *cmp, const string &query);
    bool validatePTR(const void *cmp, const vector<string> &names);
    bool compare(const void *addr, const string &cmp);

    const SPFValidatorResult &getResult();
    string getResultString();

    SPFValidator &setProtocol(Networking::IP::Protocol p);

    ~SPFValidator();
  private:
    Networking::IP::Protocol m_Protocol;
    SPFValidatorResult m_Result;
    Logger m_Logger;
  };
}

#endif
