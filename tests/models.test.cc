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
#include "../lib/models/Email.src.h"

using FSMTP::Models::EmailAddress;

// ================================
// Default email address tests
// ================================

// Parses an email address without any name or brackets
//  this should be allowed too

TEST_CASE("EmailAddress parse 'test@example.com'") {
  EmailAddress addr("test@example.com");

  REQUIRE(addr.getDomain() == "example.com");
  REQUIRE(addr.getUsername() == "test");
  REQUIRE(addr.e_Address == "test@example.com");
}

// Parses an email address without name, but with brackets
//  this should leave the name empty

TEST_CASE("EmailAddress parse '<test@example.com>'") {
  EmailAddress addr("<test@example.com>");

  REQUIRE(addr.getDomain() == "example.com");
	REQUIRE(addr.e_Name.empty());
  REQUIRE(addr.getUsername() == "test");
  REQUIRE(addr.e_Address == "test@example.com");
}

// Tests for the email address parsing with name, the extra
//  whitespace should and will be removed by the parser

TEST_CASE("EmailAddress parse '   Example Test <test@example.com>'") {
  EmailAddress addr("   Example Test <test@example.com>");

  REQUIRE(addr.getDomain() == "example.com");
  REQUIRE(addr.getUsername() == "test");
	REQUIRE(addr.e_Name == "Example Test");
  REQUIRE(addr.e_Address == "test@example.com");
}

// Tests for the email address with the name in quotes, these should
//  be removed accordingly by the parser

TEST_CASE("EmailAddress parse '\"Example Test\" <test@example.com>'") {
  EmailAddress addr("\"Example Test\" <test@example.com>");

  REQUIRE(addr.getDomain() == "example.com");
  REQUIRE(addr.getUsername() == "test");
	REQUIRE(addr.e_Name == "Example Test");
  REQUIRE(addr.e_Address == "test@example.com");
}

// Tests for the parsing of an invalid email address, and if that
//  wil throw an error

TEST_CASE("EmailAddress parse '\"Example Test\" <test@example.com'") {
  REQUIRE_THROWS(EmailAddress("\"Example Test\" <test@example.com"));
}

// ================================
// Default email list tests
// ================================

TEST_CASE("EmailAddress parse 'Example Test <test@example.com>, Hello   World <hello@world.com>'") {
	vector<EmailAddress> vec = EmailAddress::parseAddressList("Example Test <test@example.com>, Hello   World <hello@world.com>");

	REQUIRE(vec[0].e_Name == "Example Test");
	REQUIRE(vec[1].e_Name == "Hello World");

	REQUIRE(vec[0].e_Address == "test@example.com");
	REQUIRE(vec[1].e_Address == "hello@world.com");
}
