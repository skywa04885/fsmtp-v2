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
	std::string IMAPResponse::buildCapabilities(
		const std::vector<IMAPCapability> &capabilities
	)
	{
		std::string ret = "CAPABILITY ";
		for (const IMAPCapability &c : capabilities)
		{
			ret += c.c_Key;
			if (c.c_Value != nullptr)
			{
				ret += '=';
				ret += c.c_Value;
			}
			ret += ' ';
		}
		ret.pop_back();
		return ret;
	}

	/**
	 * Builds an list response
	 *
	 * @Param {const std::vector<Mailbox> &} mailboxes
	 * @Param {const std::string &} tagIndex
	 * @Return {void}
	 */
	std::string IMAPResponse::buildList(
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
				flags.push_back("HasChildren");
			else flags.push_back("HasNoChildren");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_UNMARKED))
				flags.push_back("UnMarked");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_MARKED))
				flags.push_back("Marked");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_JUNK))
				flags.push_back("Junk");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_DRAFT))
				flags.push_back("Draft");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_SENT))
				flags.push_back("Sent");
			if (BINARY_COMPARE(box.e_Flags, _MAILBOX_FLAG_ARCHIVE))
				flags.push_back("Archive");

			// Builds the result
			result << "* LIST (";
			std::size_t i = 0;
			for (const char *flag : flags)
			{
				result << flag;
				if (++i < flags.size()) result << ' ';
			}
			result << ") \"/\" " << box.e_MailboxPath.substr(2) << "\r\n";
		}

		// Adds the completed message
		result << IMAPResponse::buildCompleted(tagIndex, IMAPCommandType::ICT_LIST);
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
		switch (type)
		{
			case IMAPCommandType::ICT_LIST:
			{
				ret += "LIST";
				break;
			}
			default: throw std::runtime_error(EXCEPT_DEBUG("Type not implemented"));
		}
		ret += " completed\r\n";
		return ret;
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
		ret += " Fannst IMAP4rev1 service ready\r\n";
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
}
