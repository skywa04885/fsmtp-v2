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

#ifndef _LIB_DKIM_RECORD_H
#define _LIB_DKIM_RECORD_H

#include "../default.h"
#include "../general/Logger.src.h"
#include "../dns/Resolver.src.h"

#define _FSMTP_DKIM_RECORD_FLAG_TESTING 1
#define _FSMTP_DKIM_RECORD_FLAG_SAME_DOMAIN 2
#define _FSMTP_DKIM_RECORD_FLAG_ALLOWED_HASH_ALGO_SHA256 4
#define _FSMTP_DKIM_RECORD_FLAG_ALLOWED_HASH_ALGO_SHA1 8
#define _FSMTP_DKIM_RECORD_FLAG_ALLOWED_SERVICE_EMAIL 16

namespace FSMTP::DKIM {
  enum DKIMRecordVersion {
    RecordVersionDKIM1
  };

  const char *__dkimRecordVersionToString(DKIMRecordVersion v);

  enum DKIMRecordAlgorithm {
    RecordAlgorithmRSA
  };

  const char *__dkimRecordAlgorithmToString(DKIMRecordAlgorithm a);

  string __dkimRecordAllowedHashingAlgosToString(int32_t flags);
  string __dkiMRecordAllowedServicesToString(int32_t flags);

  class DKIMRecord {
  public:
    DKIMRecord();

    DKIMRecord &parse(const string &raw);
    DKIMRecord &print(Logger &logger);

    const char *getVersionString();
    const char *getAlgorithmString();
    
    string getAllowedHashAlgosString();
    string getAllowedServicesString();

    const string &getPublicKey();

    static DKIMRecord fromDNS(const char *query);

    ~DKIMRecord();
  private:
    DKIMRecordVersion m_Version;
    DKIMRecordAlgorithm m_Algorithm;
    int32_t m_Flags;
    string m_PublicKey;
  };
};

#endif
