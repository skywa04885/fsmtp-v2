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

#ifndef _LIB_SPF_RECORD_H
#define _LIB_SPF_RECORD_H

#include "../default.h"
#include "../dns/Resolver.src.h"
#include "../general/Logger.src.h"

#define _FSMTP_SPF_RECORD_FLAG_ALLOW_MX 1
#define _FSMTP_SPF_RECORD_FLAG_ALLOW_A 2

namespace FSMTP::SPF {
  string __spfFlagsToString(int32_t f);

  enum SPFVersion {
    SPF1
  };

  const char *__spfVersionToString(SPFVersion v);

  enum SPFPolicy {
    PolicyReject, PolicyJunk,
    PolicyNoValidation, PolicyAllowAll
  };

  const char *__spfPolicyToString(SPFPolicy p);

  class SPFRecord {
  public:
    SPFRecord();

    SPFRecord &parse(const string &raw);
    SPFRecord &print(Logger &logger);

    const char *getVersionString();
    const char *getPolicyString();
    string getFlagsString();

    const vector<string> &getIPv4s();
    const vector<string> &getIPv6s();
    const vector<string> &getMXRecords();
    const vector<string> &getARecords();
    const vector<string> &getDomains();
    const vector<string> &getPTRRecords();
    const string &getRedirect();
    bool getMXRecordsAllowed();
    bool getARecordsAllowed();
    SPFPolicy getPolicy();
    SPFVersion getVersion();

    static SPFRecord fromDNS(const char *query);

    ~SPFRecord();
  private:
    vector<string> m_AllowedIPv4s, m_AllowedIPv6s, m_AllowedMXs, 
      m_AllowedAs, m_AllowedDomains, m_AllowedPTRs;
    string m_Redirect;
    SPFVersion m_Version;
    SPFPolicy m_Policy;
    int32_t m_Flags;
  };
}

#endif
