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

#ifndef _LIB_NETWORKING_IPV4_H
#define _LIB_NETWORKING_IPV4_H

#include "../default.h"

namespace FSMTP::Networking::IPv4 {
    bool compare(const struct in_addr &address, const struct in_addr &cmp, int32_t subnetMask = -1);
	bool compare(const struct in_addr &address, string cmp);
	bool compare(const string &address, string cmp);
}

#endif
