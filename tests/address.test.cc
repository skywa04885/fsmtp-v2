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

#include <catch2/catch.hpp>
#include "../lib/networking/Address.src.h"

using FSMTP::Networking::addr_compare;
using FSMTP::Networking::AddrType;


TEST_CASE("Addr A: 192.168.1.12, B: 192.168.1.0/12") {
	REQUIRE(addr_compare("192.168.1.12", "192.168.1.0/12", AddrType::AT_IPv4));
};

TEST_CASE("Addr A: 192.168.1.13, B: 192.168.1.0/12") {
	REQUIRE(!addr_compare("192.168.1.13", "192.168.1.0/12", AddrType::AT_IPv4));
};

TEST_CASE("Addr A: 192.168.4.13, B: 192.168.0/10.12") {
	REQUIRE(addr_compare("192.168.4.12", "192.168.0/10.12", AddrType::AT_IPv4));
};

TEST_CASE("Addr A: 192.168.12.13, B: 192.168.0/10.12") {
	REQUIRE(!addr_compare("192.168.12.12", "192.168.0/10.12", AddrType::AT_IPv4));
};