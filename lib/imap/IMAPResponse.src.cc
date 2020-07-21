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

#include "IMAPResponse.src.h"

namespace FSMTP::IMAP
{
	/**
	 * Builds the list of capability's
	 *
	 * @Param {const std:string &} tagIndex
	 * @Param {const std::vector<IMAPCapability> &} capabilities
	 * @Return {std::string}
	 */
	std::string IMAPResponse::buildCapabilities(
		const std::string &tagIndex,
		const std::vector<IMAPCapability> &capabilities
	)
	{
		std::ostringstream stream;

		// Builds the capability set
		stream << "* CAPABILITY ";
		std::size_t i = 0;
		for (const IMAPCapability &c : capabilities)
		{
			stream << c.c_Key;
			if (c.c_Value != nullptr)
			{
				stream << '=';
				stream << c.c_Value;
			}
			if (++i < capabilities.size()) stream << ' ';
		}
		stream << "\r\n";

		// Adds the completed, and returns
		stream << IMAPResponse::buildCompleted(tagIndex, IMAPCommandType::ICT_CAPABILITY);
		return stream.str();
	}

	/**
	 * Builds an list response
	 *
	 * @Param {const IMAPCommandType} type
	 * @Param {const std::vector<Mailbox> &} mailboxes
	 * @Param {const std::string &} tagIndex
	 * @Return {void}
	 */
	std::string IMAPResponse::buildList(
		const IMAPCommandType type,
		const std::string &tagIndex,
		const std::vector<Mailbox> &mailboxes
	)
	{
		std::stringstream result;
		for (const Mailbox &box : mailboxes)
		{
			// Creates the flags string
			std::vector<const char *> flags;
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_HAS_SUBDIRS))
				flags.push_back("\\HasChildren");
			else flags.push_back("\\HasNoChildren");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_UNMARKED))
				flags.push_back("\\UnMarked");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_MARKED))
				flags.push_back("\\Marked");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_JUNK))
				flags.push_back("\\Junk");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_DRAFT))
				flags.push_back("\\Draft");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_SENT))
				flags.push_back("\\Sent");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_ARCHIVE))
				flags.push_back("\\Archive");

			// Builds the result
			result << "* " << IMAPResponse::getCommandStr(type) << " (";
			std::size_t i = 0;
			for (const char *flag : flags)
			{
				result << flag;
				if (++i < flags.size()) result << ' ';
			}
			result << ") \".\" \"" << box.e_MailboxPath << "\"\r\n";
		}

		// Adds the completed message
		result << IMAPResponse::buildCompleted(tagIndex, type);
		return result.str();
	}

	/**
	 * Builds the completed response
	 *
	 * @Param {const std::string &} tagIndex
	 * @Param {const IMAPCommandType} type
	 * @Return {std::string}
	 */
	std::string IMAPResponse::buildCompleted(
		const std::string &tagIndex,
		const IMAPCommandType type
	)
	{
		std::string ret = tagIndex;
		ret += " OK ";
		ret += IMAPResponse::getCommandStr(type);
		ret += " completed\r\n";
		return ret;
	}

	/**
	 * Gets the string value of a command
	 *
	 * @Param {const IMAPCommandType} type
	 * @Return {const char *}
	 */
	const char *IMAPResponse::getCommandStr(const IMAPCommandType type)
	{
		switch (type)
		{
			case IMAPCommandType::ICT_LIST: return "LIST";
			case IMAPCommandType::ICT_LSUB: return "LSUB";
			case IMAPCommandType::ICT_AUTHENTICATE: return "AUTHENTICATE";
			case IMAPCommandType::ICT_LOGIN: return "LOGIN";
			case IMAPCommandType::ICT_STARTTLS: return "STARTTLS";
			case IMAPCommandType::ICT_LOGOUT: return "LOGOUT";
			case IMAPCommandType::ICT_CAPABILITY: return "CAPABILITY";
			case IMAPCommandType::ICT_SELECT: return "SELECT";
			case IMAPCommandType::ICT_EXAMINE: return "EXAMINE";
			case IMAPCommandType::ICT_CREATE: return "CREATE";
			case IMAPCommandType::ICT_DELETE: return "DELETE";
			case IMAPCommandType::ICT_RENAME: return "RENAME";
			case IMAPCommandType::ICT_SUBSCRIBE: return "SUBSCRIBE";
			case IMAPCommandType::ICT_UNSUBSCRIBE: return "UNSUBSCRIBE";
			case IMAPCommandType::ICT_STATUS: return "STATUS";
			case IMAPCommandType::ICT_APPEND: return "APPEND";
			case IMAPCommandType::ICT_CHECK: return "CHECK";
			case IMAPCommandType::ICT_CLOSE: return "CLOSE";
			case IMAPCommandType::ICT_EXPUNGE: return "EXPUNGE";
			case IMAPCommandType::ICT_SEARCH: return "SEARCH";
			case IMAPCommandType::ICT_FETCH: return "FETCH";
			case IMAPCommandType::ICT_STORE: return "STORE";
			case IMAPCommandType::ICT_COPY: return "COPY";
			case IMAPCommandType::ICT_UID: return "UID";
			default: throw std::runtime_error(EXCEPT_DEBUG("Type not implemented"));
		}
	}

	/**
	 * Builds an formal message
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string IMAPResponse::buildGreeting(void)
	{
		std::string ret = "* OK ";
		ret += _SMTP_SERVICE_NODE_NAME;
		ret += " Fannst IMAP4rev1 service ready - max 1.800.000ms\r\n";
		return ret;
	}

	/**
	 * Builds an bad message
	 *
	 * @Param {const std::string &} reason
	 * @Param {const std::string &} index
	 * @Return {std::string}
	 */
	std::string IMAPResponse::buildBad(const std::string index, const std::string &reason)
	{
		std::string ret = index;
		ret += " BAD ";
		ret += reason;
		ret += "\r\n";
		return ret;
	}

	/**
	 * Builds an no message
	 *
	 * @Param {const std::string &} reason
	 * @Param {const std::string &} index
	 * @Return {std::string}
	 */
	std::string IMAPResponse::buildNo(const std::string index, const std::string &reason)
	{
		std::string ret = index;
		ret += " NO ";
		ret += reason;
		ret += "\r\n";
		return ret;
	}

	/**
	 * Builds an message with confirmation, stucture:
	 * `
	 * * Ready to start TLS
	 * AA0 OK STARTTLS completed
	 * ` 
	 *
	 * @Param {const IMAPCommandType} type
	 * @Return {std::string}
	 */
	std::string IMAPResponse::buildMessageWithConfirm(const IMAPCommandType type)
	{
		// TODO: make this
		return "";
	}
}
