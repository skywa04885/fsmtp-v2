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
	/*
	 * Compares to addresses with for example support for ranges
	 *  which is used in SPF.
	 */
	bool addr_compare(const string &a, const string &b, const AddrType type) {
		if (type == AddrType::AT_IPv4) {
			vector<string> ASegments = {};
			string segment;

			// Gets the default segments from the a string
			//  this will not contain any ranges
			{
				stringstream stream(a);
				while (getline(stream, segment, '.')) {
					ASegments.push_back(segment);
				}
			}

			// Starts comparing the segments, this will
			//  be done using a range stored in the b
			//  address
			stringstream stream(b);
			size_t i = 0;
			while (getline(stream, segment, '.')) {
				auto &currentASegment = ASegments[i++];

				if (segment.find_first_of('/') == string::npos) {
					if (segment == currentASegment) continue;
					else return false;
				} else {
					int32_t rangeFrom = stoi(segment.substr(0, segment.find_first_of('/')));
					int32_t rangeTo = stoi(segment.substr(segment.find_first_of('/') + 1));
					int32_t numberOfA = stoi(currentASegment);

					if (numberOfA >= rangeFrom && numberOfA <= rangeTo) continue;
					else return false;
				}
			}

			return true;
		} else {

		}
	}	
}