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

#include "Address.src.h"

namespace FSMTP::Networking {
	bool isAddress(const string &a, AddrType type) {
		auto isNumber = [](const string &str) {
			return !str.empty() && (str.find_first_not_of("[0123456789]") == string::npos);
		};

		if (type == AddrType::AT_IPv4) {
			vector<string> segments = {};

			// Parses the address into segments, so we can later
			//  check if it really is an valid ip address
			size_t start = 0, end = a.find_first_of('.');
			for (;;) {
				segments.push_back(a.substr(start, end - start));
				if (end == string::npos) break;

				start = end + 1;
				end = a.find_first_of('.', start);
			}

			// Checks if the address is a valid IPv4 address
			if (segments.size() != 4) return false;
			for (auto &seg : segments) {
				int32_t segNumber = stoi(seg);
				if (!isNumber(seg) || segNumber < 0 || segNumber > 255) return false;
			}
			return true;
		} else {
			return true;
		}
	}

	uint32_t bin_from_ipv4(const string &ip) {
		uint32_t result = 0x0;
		stringstream stream(ip);
		string segment;

		size_t i = 0;
		while (getline(stream, segment, '.')) {
			if (i > 4) break;
			result |= stoi(segment) << (32 - ++i * 8);
		}

		return result;
	}

	uint128_t bin_from_ipv6(const string &ip) {
		uint128_t result;
		memset(&result, 0, sizeof(result));
		stringstream stream(ip);
		string segment;

		auto decodeC = [](const char a) {
			if (a >= 48 && a <= 57) return a - 48;

			switch (tolower(a)) {
				case 'a': return 10;
				case 'b': return 11;
				case 'c': return 12;
				case 'd': return 13;
				case 'e': return 14;
				case 'f': return 15;
			}
		};

		size_t i = 0;
		while (getline(stream, segment, ':')) {

		}

		return result;
	}

	/*
	 * Compares to addresses with for example support for ranges
	 *  which is used in SPF.
	 */
	bool addr_compare(const string &a, const string &b, const AddrType type) {
		if (type == AddrType::AT_IPv4) {

			if (b.find_first_of('/') != string::npos) {
				uint32_t aBinary = bin_from_ipv4(a);
				uint32_t bBinary = bin_from_ipv4(b);

				int32_t bCidr = stoi(b.substr(b.find_first_of('/') + 1));

				aBinary >>= 32 - bCidr;
				bBinary >>= 32 - bCidr;
				return aBinary == bBinary;
			} else return a == b;
		} else {
			// Checks if the address we need to compare to contains
			//  an subnet, if so we need to treat it differently,
			//  else just compare as strings
			if (b.find_first_of('/') != string::npos) {

			} else return a == b;
		}
	}	
}