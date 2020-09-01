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

SPFRecord::SPFRecord(const string &raw) {
	this->parse(const string &raw);
}

void SPFRecord::parse(const string &raw) {
	vector<string> parts = {};
	stringstream stream(raw);
	string part;

	while (getline(stream, part, ' ')) {
		if (part.empty()) continue;
		else parts.push_back(part);
	}

	for_each(parts.begin(), parts.end(), [&](auto &part) {
			if (part[0] == '~') {
				if (part.substr(1) == "all") {

				}
			} else if (part[1] == '-') {
				if (part.substr(1) == "all") {

				}
			} else if (part == "mx") {

			} else if (part == "a") {

			} else {
				string key, val;
				size_t sep;

				if ((sep = part.find_first_of(':')) == string::npos) {
					throw runtime_error(EXCEPT_DEBUG("Invalid header: key value pair not existing"));
				}

				key = part.substr(0, sep);
				val = part.substr(++sep);
			
				if (key == "a") {

				} else if (key == "mx") {

				} else if (key == "ipv4") {

				} else if (key == "include") {

				} else throw runtime_error(EXCEPT_DEBUG("Invalid header: key unknown !"));
			}
	});
}

