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

#ifndef _LIB_DKIM_SIGNER_H
#define _LIB_DKIM_SIGNER_H

#include "DKIMHeader.src.h"
#include "DKIMCanonicalization.src.h"
#include "DKIMHashes.src.h"

#include "../general/Logger.src.h"
#include "../parsers/mimev2.src.h"

namespace FSMTP::DKIM {
	struct DKIMSignerConfig {
		string domain, keySelector, privateKeyPath;
		int64_t signTime, expireTime;
		DKIMHeaderCanonAlgPair algorithmPair;
		DKIMHeaderAlgorithm signAlgorithm;
		vector<string> headerFilter;
	};

	class DKIMSigner {
	public:
		DKIMSigner();

		DKIMSigner &setDomain(const string &domain);
		DKIMSigner &setKeySelector(const string &keySelector);
		DKIMSigner &setPrivateKeyPath(const string &privateKeyPath);
		DKIMSigner &setSignTime(int64_t signTime);
		DKIMSigner &setExpireTime(int64_t expireTime);
		DKIMSigner &setAlgoPair(DKIMHeaderCanonAlgPair algorithmPair);
		DKIMSigner &setSignAlgo(DKIMHeaderAlgorithm signAlgorithm);
		DKIMSigner &setHeaderFilter(const vector<string> &filter);
		DKIMSigner &headerFilterPush(const string &header);

		DKIMSigner &setConfig(const DKIMSignerConfig &config);

		DKIMSigner &sign(const string &mime);

		const DKIMHeader &getResult() const;

		~DKIMSigner();
	protected:
		void generateBodyHash(const string &body);
		void generateSignature(const string &headers);
	private:
		DKIMSignerConfig m_Config;
		DKIMHeader m_Result;
		Logger m_Logger;
	};
}

#endif
