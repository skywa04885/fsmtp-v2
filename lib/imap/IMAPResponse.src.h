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
#include "IMAPCommand.src.h"

namespace FSMTP::IMAP
{
	typedef struct
	{
		const char *c_Key;
		const char *c_Value;
	} IMAPCapability;

	class IMAPResponse
	{
	public:
		/**
		 * Builds an list response
		 *
		 * @Param {const std::vector<Mailbox> &} mailboxes
		 * @Param {const std::string &} tagIndex
		 * @Return {void}
		 */
		static std::string buildList(
			const std::string &tagIndex,
			const std::vector<Mailbox> &mailboxes
		);

		/**
		 * Builds the completed response
		 *
		 * @Param {const std::string &} tagIndex
		 * @Param {const IMAPCommandType} type
		 * @Return {std::string}
		 */
		static std::string buildCompleted(
			const std::string &tagIndex,
			const IMAPCommandType type
		);

		static std::string buildCapabilities(
			const std::vector<IMAPCapability> &capabilities
		);

		/**
		 * Builds an formal message
		 *
		 * @Param {void}
		 * @Return {std::string}
		 */
		static std::string buildGreeting(void);

		/**
		 * Builds an bad message
		 *
		 * @Param {const std::string &} reason
	 	 * @Param {const std::string &} index
		 * @Return {std::string}
		 */
		static std::string buildBad(const std::string index, const std::string &reason);
	};
}