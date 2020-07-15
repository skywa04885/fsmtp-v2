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

#pragma once

#include <string>
#include <random>
#include <iostream>

#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>

namespace FSMTP
{
	/**
	 * Verifies an password with existing comparable password
	 *
	 * @Param {const std::string &} password
	 * @Param {const std::string &} comparable
	 * @Return {bool}
	 */
	bool passwordVerify(const std::string &password, const std::string &compared);

	/**
	 * Generates salt and hashes an password
	 *
	 * @Param {const std::string &} password
	 * @Return {std::string}
	 */
	std::string passwordHash(const std::string &password);

	/**
	 * Only hashes an password to base64, give it some existing salt
	 *
	 * @Param {const std::string &} password
	 * @Param {const std::string &} salt
	 * @Return {std::string}
	 */
	std::string passwordHashOnly(const std::string &password, const std::string &salt);
}