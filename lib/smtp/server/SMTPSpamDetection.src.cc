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

#include "SMTPSpamDetection.src.h"

namespace FSMTP::Server::SpamDetection {
	bool checkSpamhaus(string ip) {
		// Reverses the remote address, this is required since zen some or another
		//  way works like this
		vector<string> segments = {};
		stringstream ss(ip);
		string segment;

		while(getline(ss, segment, '.')) {
			segments.push_back(segment);
		}

		reverse(segments.begin(), segments.end());

		// Builds the final address, with the reverse and the zen address
		//  appended to it, when this is done we resolve the DNS records

		string address;
		for_each(segments.begin(), segments.end(), [&](auto &seg) {
			address += seg;
			address += '.';
		});
		address += "zen.spamhaus.org";

		try {
			vector<Record> records = resolveDNSRecords(address, RecordType::RT_A);
			return true;
		} catch (...) {
			return false;
		}
	}
}
