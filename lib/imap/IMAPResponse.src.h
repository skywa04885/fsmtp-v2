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

	typedef enum
	{
		BVT_LIST = 0,
		BVT_STRING,
		BVT_ATOM,
		BVT_BRACKETS,
		BVT_OK,
		BVT_BAD,
		BVT_NO,
		BVT_COMPLETED,
		BVT_INT32,
		BVT_COMMAND
	} BuildLineSectionType;

	typedef struct
	{
		BuildLineSectionType b_Type;
		const void *b_Value;
	} BuildLineSection;

	typedef struct
	{
		const char *b_Index;
		std::vector<BuildLineSection> b_Sections;
	} BuildLine;

	class IMAPResponse
	{
	public:
		/**
		 * Builds an list response
		 *
		 * @Param {const IMAPCommandType} type
		 * @Param {const std::vector<Mailbox> &} mailboxes
		 * @Param {const std::string &} tagIndex
		 * @Return {void}
		 */
		static std::string buildList(
			const IMAPCommandType type,
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

		/**
		 * Builds the list of capability's
		 *
		 * @Param {const std:string &} tagIndex
		 * @Param {const std::vector<IMAPCapability> &} capabilities
		 * @Return {std::string}
		 */
		static std::string buildCapabilities(
			const std::string &tagIndex,
			const std::vector<IMAPCapability> &capabilities
		);

		/**
		 * Gets the string value of a command
		 *
		 * @Param {const IMAPCommandType} type
		 * @Return {const char *}
		 */
		static const char *getCommandStr(const IMAPCommandType type);

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

		/**
		 * Builds an no message
		 *
		 * @Param {const std::string &} reason
	 	 * @Param {const std::string &} index
		 * @Return {std::string}
		 */
		static std::string buildNo(const std::string index, const std::string &reason);

		/**
		 * Builds the select information header, when an mailbox is selected
		 *
		 * @Param {const std::string &} index
		 * @Param {const MailboxStatys &} status
		 * @Return {std::string}
		 */
		static std::string buildSelectInformation(
			const std::string &index,
			const MailboxStatus &status
		);

		/**
		 * Builds an list of email flags
		 *
		 * @Param {const int32_t} flags
		 * @return {std::string}
		 */
		static std::string buildEmailFlagList(const int32_t flags);

		/**
		 * Builds an response message
		 *
		 * @Param {const std::vector<BuildLine> &} lines
		 */
		static std::string build(const std::vector<BuildLine> &lines);

		/**
		 * Builds th emailbox flags
		 *
		 * @Param {const int32_t flags}
		 * @Return {std::vector<const char *>}
		 */
		static std::vector<const char *> buildMailboxFlags(const int32_t flags);
	};
}