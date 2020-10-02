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

#ifndef _LIB_DKIM_HEADER_H
#define _LIB_DKIM_HEADER_H

#include "../default.h"
#include "../general/Logger.src.h"
#include "../builders/mimev2.src.h"

namespace FSMTP::DKIM {
  enum DKIMHeaderVersion {
    HeaderVersionDKIM1
  };

  const char *__dkimHeaderVersionToString(DKIMHeaderVersion v);

  enum DKIMHeaderCanonAlgPair {
    RelaxedRelaxed, RelaxedSimple, SimpleRelaxed,
    SimpleSimple
  };

  const char *__dkimHeaderCanonAlgPairToString(DKIMHeaderCanonAlgPair a);

  enum DKIMHeaderAlgorithm {
    HeaderAlgoritmRSA_SHA256, HeaderAlgorithmRSA_SHA1
  };

  const char *__dkimHeaderAlgToString(DKIMHeaderAlgorithm a);

  class DKIMHeader {
  public:
    DKIMHeader();

    DKIMHeader &parse(const string &raw);
    DKIMHeader &print(Logger &logger);

    const char *getVersionString();
    const char *getCanonAlgPairString();
    const char *getHeaderAlgString();

    string getHeaderString() const;

    DKIMHeaderCanonAlgPair getCanonAlgorithmPair();
    DKIMHeaderAlgorithm getHeaderAlgorithm();
    const string &getBodyHash();
    const string &getKeySelector();
    const string &getDomain();
    const string &getSignature();
    const vector<string> &getHeaders();

    DKIMHeader &setBodyHash(const string &bodyHash);
    DKIMHeader &setSignature(const string &signature);
    DKIMHeader &setDomain(const string &domain);
    DKIMHeader &setKeySelector(const string &keySelector);
    DKIMHeader &setHeaders(const vector<string> &headers);
    DKIMHeader &setCanonAlgoPair(const DKIMHeaderCanonAlgPair &canonAlgo);
    DKIMHeader &setHeaderAlgo(const DKIMHeaderAlgorithm &algo);

    string build() const;

    ~DKIMHeader();
  private:
    string m_Signature, m_BodyHash, m_Domain, m_KeySelector;
    vector<string> m_Headers;
    DKIMHeaderVersion m_Version;
    DKIMHeaderCanonAlgPair m_CanonAlgoPair;
    DKIMHeaderAlgorithm m_HeaderAlgorithm;
  };
}

#endif
