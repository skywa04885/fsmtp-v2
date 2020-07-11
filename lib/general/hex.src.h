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

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

namespace FSMTP::Encoding::HEX
{
	/**
	 * Turns an hex char to valid number
	 *
	 * @Param {const char} c
	 * @Return {char}
	 */
	char _reverseDict(const char c);

	/**
	 * Decodes an string of hexadecimal characters
	 *
	 * @Param {const std::string &} raw
	 * @Param {std::string &} ret
	 * @Return {void}
	 */
	void decode(const std::string &raw, std::string &ret);

	/**
	 * Encodes an raw string into an hexadecimal string\
	 *
	 * @Param {const std::string &} raw
	 * @Param {std::string &} ret
	 * @Return {void}
	 */
	void encode(const std::string &raw, std::string &ret);
}