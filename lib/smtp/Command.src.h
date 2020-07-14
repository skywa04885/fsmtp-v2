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

#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include "../general/cleanup.src.h"

using namespace FSMTP::Cleanup;

namespace FSMTP::SMTP
{
	typedef enum : uint32_t {
		CCT_HELO = 0,
		CCT_EHLO,
		CCT_START_TLS,
		CCT_MAIL_FROM,
		CCT_RCPT_TO,
		CCT_DATA,
		CCT_QUIT,
		CCT_UNKNOWN
	} ClientCommandType;

	class ClientCommand
	{
	public:
		/**
		 * Default constructor for the ClientCommand
		 *
		 * @Param {ClientCommandType &} c_CommandType
		 * @Param {std::vector<std::string> &} c_Arguments
		 * @Return void
		 */
		ClientCommand(
			const ClientCommandType &c_CommandType,
			const std::vector<std::string> &c_Arguments);

		/**
		 * Default empty constructor for the ClientCommand
		 *
		 * @Param void
		 * @Return void
		 */
		ClientCommand();

		/**
		 * Default constructor which actually
		 * - parses an existing command
		 *
		 * @Param {std::string &} raw
		 * @Return void
		 */
		ClientCommand(const std::string &raw);

		/**
		 * Builds the client command
		 *
		 * @Param {void}
		 * @Return {std::string}
		 */
		std::string build(void);
		
		ClientCommandType c_CommandType;
		std::vector<std::string> c_Arguments;
	};
}
