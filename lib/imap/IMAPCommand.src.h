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

#include "IMAPResponse.src.h"

namespace FSMTP::IMAP
{
	typedef enum : uint32_t
	{
		ICT_CAPABILITY = 0,
		ICT_UNKNOWN,
		ICT_LOGOUT,
		ICT_LOGIN
	} IMAPCommandType;

	class IMAPCommand
	{
	public:
		/**
		 * Default emty constructor
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit IMAPCommand(void);

		/**
		 * Parses an IMAP command
		 *
		 * @Param {const std::string &} raw
		 * @Return {void}
		 */
		IMAPCommand(const std::string &raw);

		/**
		 * Parses an IMAP command
		 *
		 * @Param {const std::string &} raw
		 * @Return {void}
		 */
		void parse(const std::string &raw);
	
		std::string c_Index;
		IMAPCommandType c_Type;
		std::vector<std::string> c_Args;
	};
}
