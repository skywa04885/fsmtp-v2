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


#include "IPv4.src.h"

namespace FSMTP::Networking::IPv4 {
    inline int32_t __parseSubnetMask(string &cmp) {
        int32_t result = -1;
        
        size_t sep = cmp.find_first_of('/');
        if (sep != string::npos) {
            result = stoi(cmp.substr(sep + 1));
            cmp = cmp.substr(0, sep);
        }

        return result;
    }

    inline void __ipv4FromString(const string &addr, struct in_addr &res) {
        if (inet_pton(AF_INET, addr.c_str(), &res) != 1)
            throw runtime_error(EXCEPT_DEBUG(
                string("Invalid IPv4 address: '") + addr + '\''));
    }

    bool compare(const struct in_addr &address, const struct in_addr &cmp, int32_t subnetMask) {
        uint32_t binaryAddress = address.s_addr, binaryCmp = cmp.s_addr;

        if (subnetMask != -1) {
            binaryAddress >>= 32 - subnetMask;
            binaryCmp >>= 32 - subnetMask;
        }

        return (binaryAddress == binaryCmp);
    }
	
    bool compare(const struct in_addr &address, string cmp) {
        int32_t subnetMask = __parseSubnetMask(cmp);
        struct in_addr parsedCmp;

        __ipv4FromString(cmp, parsedCmp);
        return compare(address, parsedCmp, subnetMask);
    }
	
    bool compare(const string &address, string cmp) {
        int32_t subnetMask = __parseSubnetMask(cmp);
        struct in_addr parsedAddr, parsedCmp;

        __ipv4FromString(address, parsedAddr);
        __ipv4FromString(cmp, parsedCmp);
        return compare(parsedAddr, parsedCmp, subnetMask);
    }
}
