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
#include "../general/Logger.src.h"
#include "../general/cleanup.src.h"

#define _SPF_FLAG_SOFTFAIL_ALL 1
#define _SPF_FLAG_DENY_ALL 2
#define _SPF_FLAG_ALLOW_MX 4
#define _SPF_FLAG_ALLOW_A 8
#define _SPF_FLAG_ALLOW_ALL 16
#define _SPF_FLAG_ALLOW_NO_FURTHER_CHECKS 32
#define _SPF_FLAG_DEPRECATED 64

using namespace FSMTP::Cleanup;

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
		bool getAllowNoQuestionsAsked() const;
		bool getDeprecated() const;
		
		/**
		 * Gets the set of allowed domains specified in the header
		 */
		const vector<string>& getAllowedDomains() const;

		/**
		 * Gets the set of allowed ipv4 addresses specified in the header
		 */
		const vector<string>& getAllowedIPV4s() const;

		/**
		 * Gets the set of allowed A record domains. Domains which A recourd
		 *  should be used as valid.
		 */
		const vector<string>& getAllowedADomains() const;

		/**
		 * Gets the set of allowed MX record domains, the MX records addresses
		 *  will be marked as allowed.
		 */
		const vector<string>& getAllowedMXDomains() const;

		/**
		 * Returns true if we need to redirect
		 */
		bool shouldRedirect();

		/**
		 * Returns the redirect uri
		 */
		string &getRedirectURI();

		void print(Logger &logger);
	private:
		string s_Redirect;
		uint32_t s_Flags;
		vector<string> s_AllowedIPV4s;
		vector<string> s_AllowedIPV6s;
		vector<string> s_AllowedADomains;
		vector<string> s_AllowedMXDomains;
		vector<string> s_AllowedDomains;
	};
}

#endif
