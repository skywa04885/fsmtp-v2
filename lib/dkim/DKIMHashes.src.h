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
	/**
	 * Creates an SHA256-Base64 Hash using openssl
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string sha256base64(const std::string &raw);

	/**
	 * Generates the RSA-SHA256 signature and returns it in
	 * - base64 format
	 *
	 * @param {const std::string &} raw
	 * @Param {const char *} privateKeyFile
	 * @return {std::string}
	 */
	std::string RSASha256generateSignature(
		const std::string &raw, 
		const char *privateKeyFile
	);
}