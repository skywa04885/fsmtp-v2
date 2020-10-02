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

/*
=>
	All options and decisions for the building of this file is available
	in [RFC6476](https://tools.ietf.org/html/rfc6376)
=>
*/

#pragma once

#include "../default.h"

namespace FSMTP::DKIM::Hashes
{
	string sha256base64(const std::string &raw);
	string sha1base64(const string &raw);

	string RSAShagenerateSignature(const string &raw, const char *pkey, const EVP_MD *type);

	bool RSAverify(const string &signature, const string &raw, const string &pubKey, const EVP_MD *type);
}