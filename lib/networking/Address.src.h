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

#ifndef _LIB_NET_ADDR_H
#define _LIB_DNS_ADDR_H

#include "../default.h"
#include "../general/hex.src.h"

namespace FSMTP::Networking {
	typedef enum {
		AT_IPv4,
		AT_IPv6
	} AddrType;

	/**
	 * Compares to addresses with for example support for ranges
	 *  which is used in SPF.
	 */
	bool addr_compare(const string &a, const string &b, const AddrType type);

	uint32_t bin_from_ipv4(const string &ip);
	uint128_t bin_from_ipv6(const string &ip);
}

#endif