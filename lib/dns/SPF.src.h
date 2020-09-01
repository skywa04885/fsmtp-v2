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


#ifndef _LIB_DNS_SPF_H
#define _LIB_DNS_SPF_H

#include "../default.h"

namespace FSMTP::DNS::SPF {
	class SPFRecord {
	public:
		SPFRecord(const string &raw);

		void parse(const string &raw);

		bool getARecordsAllowed() const;
		bool getMXRecordsAllowed() const;
		bool ipv4Allowed(const string &ipv4) const;
		bool getSoftfailFlag() const;
		bool getDenyFlag() const;

		/**
		 * Gets the set of allowed domains specified in the header
		 */
		vector<string> getAllowedDomains() const;

		/**
		 * Gets the set of allowed ipv4 addresses specified in the header
		 */
		vector<string> getAllowedIPV4s() const;

		/**
		 * Gets the set of allowed A record domains. Domains which A recourd
		 *  should be used as valid.
		 */
		vector<string> getAllowedADomains() const;

		/**
		 * Gets the set of allowed MX record domains, the MX records addresses
		 *  will be marked as allowed.
		 */
		vector<string> getAllowedMXDomains() const;
	};
}

#endif
