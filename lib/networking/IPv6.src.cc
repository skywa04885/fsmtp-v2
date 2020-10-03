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

#include "IPv6.src.h"

namespace FSMTP::Networking::IPv6 {
    inline int32_t __parseSubnetMask(string &cmp) {
        int32_t result = -1;

        size_t sep = cmp.find_first_of('/');
        if (sep != string::npos) {
            result = stoi(cmp.substr(sep + 1));
            cmp = cmp.substr(0, sep);
        }

        return result;
    }

    inline boost::multiprecision::uint128_t __ipv6ToBinary(const struct in6_addr &a) {
        boost::multiprecision::uint128_t res = 0x0;
        for (uint8_t i = 0; i < 16; ++i) {
            res |= a.s6_addr[i];
            if (i != 15) res <<= 8;
        }

        return res;
    }

    inline void __ipv6FromString(const string &addr, struct in6_addr &res) {
        if (inet_pton(AF_INET6, addr.c_str(), &res) != 1)
            throw runtime_error(EXCEPT_DEBUG(
                string("Invalid IPv6 address: '") + addr + '\''));
    }

    bool compare(const struct in6_addr &address, const struct in6_addr &cmp, int32_t subnetMask) {
        auto binaryAddress = __ipv6ToBinary(address), binaryCmp = __ipv6ToBinary(cmp);

        if (subnetMask != -1) {
            binaryAddress <<= 128 - subnetMask;
            binaryCmp <<= 128 - subnetMask;
        }

        return (binaryAddress == binaryCmp);
    }

	bool compare(const struct in6_addr &address, string cmp) {
        int32_t subnetMask = __parseSubnetMask(cmp);
        struct in6_addr parsedCmp;
        
        __ipv6FromString(cmp, parsedCmp);
        return compare(address, parsedCmp, subnetMask);
    }
	
    bool compare(const string &address, string cmp) {
        int32_t subnetMask = __parseSubnetMask(cmp);
        struct in6_addr parsedAddr, parsedCmp;

        __ipv6FromString(address, parsedAddr);
        __ipv6FromString(cmp, parsedCmp);
        return compare(parsedAddr, parsedCmp, subnetMask);
    }

}
