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

#include "SPF.src.h"

using namespace FSMTP::DNS::SPF;

SPFRecord::SPFRecord(const string &raw):
	s_Flags(0x0) {
	this->parse(raw);
}

void SPFRecord::parse(const string &raw) {
	vector<string> parts = {};
	stringstream stream(raw);
	string part;

	// Splits the header into parts, which we 
	//  later will parse.
	while (getline(stream, part, ' ')) {
		string cleanPart;
		reduceWhitespace(part, cleanPart);
		removeFirstAndLastWhite(cleanPart);

		if (cleanPart.empty()) continue;
		else parts.push_back(cleanPart);
	}

	// Parses the parts, after which we return
	for_each(parts.begin(), parts.end(), [&](auto &part) {
			if (part[0] == 'v') return;
			else if (part[0] == '~') {
				if (part.substr(1) == "all") this->s_Flags |= _SPF_FLAG_SOFTFAIL_ALL;
			} else if (part[0] == '-') {
				if (part.substr(1) == "all") this->s_Flags |= _SPF_FLAG_DENY_ALL;
			} else if (part[0] == '+') {
				if (part.substr(1) == "all") this->s_Flags |= _SPF_FLAG_ALLOW_ALL;
			} else if (part[0] == '?') {
				if (part.substr(1) == "all") this->s_Flags |= _SPF_FLAG_ALLOW_NO_FURTHER_CHECKS;
			} else if (part == "mx") this->s_Flags |= _SPF_FLAG_ALLOW_MX;
			else if (part == "a") this->s_Flags |= _SPF_FLAG_ALLOW_A;
			else {
				string key, val;
				size_t sep;

				if ((sep = part.find_first_of(':')) == string::npos) {
					// Checks if the separator is =
					if ((sep = part.find_first_of('=')) == string::npos)
						throw runtime_error(EXCEPT_DEBUG(
							string("Invalid header: key value pair not existing for: ") + part)
						);
				}

				key = part.substr(0, sep);
				val = part.substr(++sep);
			
				if (key == "a") this->s_AllowedADomains.push_back(val);
				else if (key == "mx") this->s_AllowedMXDomains.push_back(val);
				else if (key == "ip4") this->s_AllowedIPV4s.push_back(val);
				else if (key == "ip6") this->s_AllowedIPV6s.push_back(val);
				else if (key == "ptr") this->s_Flags |= _SPF_FLAG_DEPRECATED;
				else if (key == "include") this->s_AllowedDomains.push_back(val);
				else if (key == "redirect") this->s_Redirect = val;
				else return; // Just ignore
			}
	});
}

void SPFRecord::print(Logger &logger) {
	logger << "SPFRecord: " << ENDL;
	logger << " - Redirect: " << 
		(this->s_Redirect.empty() ? "No" : string("Yes: ") + this->s_Redirect) << ENDL;
	logger << " - Flags: " << bitset<32>(this->s_Flags) << ':' << ENDL;

	// Prints the flags in a meaningfull way ( with strings )
	vector<const char *> flags = {};
	if (BINARY_COMPARE(this->s_Flags, _SPF_FLAG_ALLOW_A)) flags.push_back("Allow A Records");
	if (BINARY_COMPARE(this->s_Flags, _SPF_FLAG_ALLOW_MX)) flags.push_back("Allow MX Records");
	if (BINARY_COMPARE(this->s_Flags, _SPF_FLAG_DENY_ALL)) flags.push_back("Deny all");
	if (BINARY_COMPARE(this->s_Flags, _SPF_FLAG_SOFTFAIL_ALL)) flags.push_back("Softfail");
	if (BINARY_COMPARE(this->s_Flags, _SPF_FLAG_ALLOW_ALL)) flags.push_back("Allow all");
	if (BINARY_COMPARE(this->s_Flags, _SPF_FLAG_ALLOW_NO_FURTHER_CHECKS)) flags.push_back("No further checks");

	size_t i = 0;
	for_each(flags.begin(), flags.end(), [&](auto &flag) {
			logger << '\t' << i++ << ": " << flag << ENDL;
	});

	// Prints the allowed IP's and domains
	logger << " - Allowed IPv4's: " << ENDL;
	i = 0;
	for_each(this->s_AllowedIPV4s.begin(), this->s_AllowedIPV4s.end(), [&](auto &ip) {
			logger << '\t' << i++ << ": " << ip << ENDL;
	});
	
	logger << " - Allowed IPv6's: " << ENDL;
	i = 0;
	for_each(this->s_AllowedIPV6s.begin(), this->s_AllowedIPV6s.end(), [&](auto &ip) {
			logger << '\t' << i++ << ": " << ip << ENDL;
	});

	logger << " - Allowed Domains: " << ENDL;
	i = 0;
	for_each(this->s_AllowedDomains.begin(), this->s_AllowedDomains.end(), [&](auto &ip) {
			logger << '\t' << i++ << ": " << ip << ENDL;
	});

	logger << " - Allowed A Records from domains: " << ENDL;
	i = 0;
	for_each(this->s_AllowedADomains.begin(), this->s_AllowedADomains.end(), [&](auto &ip) {
			logger << '\t' << i++ << ": " << ip << ENDL;
	});

	logger << " - Allowed MX Records from domains: " << ENDL;
	i = 0;
	for_each(this->s_AllowedMXDomains.begin(), this->s_AllowedMXDomains.end(), [&](auto &ip) {
			logger << '\t' << i++ << ": " << ip << ENDL;
	});
}

string &SPFRecord::getRedirectURI() {
	return this->s_Redirect;
}

const vector<string> &SPFRecord::getAllowedDomains() const {
	return this->s_AllowedDomains;
}

bool SPFRecord::shouldRedirect() {
	return !this->s_Redirect.empty();
}

const vector<string> &SPFRecord::getAllowedIPV4s() const {
	return this->s_AllowedIPV4s;
}

bool SPFRecord::getARecordsAllowed() const {
	return BINARY_COMPARE(this->s_Flags, _SPF_FLAG_ALLOW_A);
}

bool SPFRecord::getMXRecordsAllowed() const {
	return BINARY_COMPARE(this->s_Flags, _SPF_FLAG_ALLOW_MX);
}

bool SPFRecord::getAllowNoQuestionsAsked() const {
	return BINARY_COMPARE(this->s_Flags, _SPF_FLAG_ALLOW_ALL);
}

bool SPFRecord::getDeprecated() const {
	return BINARY_COMPARE(this->s_Flags, _SPF_FLAG_DEPRECATED);
}