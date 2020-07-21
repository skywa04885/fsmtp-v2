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

#include "IMAP.src.h"

namespace FSMTP::IMAP
{
	typedef enum : uint32_t
	{
		// Non authenticated commands
		ICT_STARTTLS = 0,
		ICT_AUTHENTICATE,
		ICT_LOGIN,
		ICT_LOGOUT,
		ICT_CAPABILITY,
		// Authenticated commands
		ICT_SELECT,
		ICT_EXAMINE,
		ICT_CREATE,
		ICT_DELETE,
		ICT_RENAME,
		ICT_SUBSCRIBE,
		ICT_UNSUBSCRIBE,
		ICT_LIST,
		ICT_LSUB,
		ICT_STATUS,
		ICT_APPEND,
		// Selected state
		ICT_CHECK,
		ICT_CLOSE,
		ICT_EXPUNGE,
		ICT_SEARCH,
		ICT_FETCH,
		ICT_STORE,
		ICT_COPY,
		ICT_UID,
		// Other
		ICT_UNKNOWN,
		ICT_NOOP
	} IMAPCommandType;

	typedef enum : uint8_t
	{
		IAT_ATOM = 0,
		IAT_NUMBER,
		IAT_STRING,
		IAT_NIL
	} IMAPCommandArgType;

	typedef struct
	{
		IMAPCommandArgType a_Type;
		std::variant<std::string, int32_t> a_Value;
	} IMAPCommandArg;

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

		/**
		 * Gets and sets the type of the command
		 *
		 * @Param {std::string} command
		 * @Return {void}
		 */
		void getType(std::string command);
	
		std::string c_Index;
		IMAPCommandType c_Type;
		std::vector<IMAPCommandArg> c_Args;
	};
}
