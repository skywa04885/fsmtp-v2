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
		static IMAPCommandType type = IMAPCommandType::ICT_CAPABILITY;

		// Builds the capability list
		std::string capaStr;
		std::size_t i = 0;
		for (auto &capability : capabilities)
		{
			capaStr += capability.c_Key;
			if (capability.c_Value != nullptr)
			{
				capaStr += '=';
				capaStr += capability.c_Value;
			}
			if (++i < capabilities.size()) capaStr += ' ';
		}

		// Builds the response
		return IMAPResponse::build({
			BuildLine {
				nullptr,
				{
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>("CAPABILITY")
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>(capaStr.c_str())
					}
				}
			},
			BuildLine {
				tagIndex.c_str(),
				{
					BuildLineSection {
						BVT_OK, nullptr
					},
					BuildLineSection {
						BVT_COMPLETED, reinterpret_cast<const void *>(&type)
					}
				}
			}
		});
	}


	/**
	 * Builds th emailbox flags
	 *
	 * @Param {const int32_t flags}
	 * @Return {std::vector<const char *>}
	 */
	std::vector<const char *> IMAPResponse::buildMailboxFlags(const int32_t flags)
	{
		std::vector<const char *> res;
		res.reserve(_MAILBOX_FLAG_COUNT);

		// Compares and creates the flags
		if (BINARY_COMPARE(flags, _MAILBOX_FLAG_HAS_SUBDIRS))
			res.push_back("\\HasChildren");
		else
			res.push_back("\\HasNoChildren");
		if (BINARY_COMPARE(flags, _MAILBOX_FLAG_UNMARKED))
			res.push_back("\\UnMarked");
		if (BINARY_COMPARE(flags, _MAILBOX_FLAG_MARKED))
			res.push_back("\\Marked");
		if (BINARY_COMPARE(flags, _MAILBOX_FLAG_JUNK))
			res.push_back("\\Junk");
		if (BINARY_COMPARE(flags, _MAILBOX_FLAG_DRAFT))
			res.push_back("\\Draft");
		if (BINARY_COMPARE(flags, _MAILBOX_FLAG_SENT))
			res.push_back("\\Sent");
		if (BINARY_COMPARE(flags, _MAILBOX_FLAG_ARCHIVE))
			res.push_back("\\Archive");

		return res;
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
		// Builds the mailboxes list
		std::string result;
		for (auto &mailbox : mailboxes)
		{
			std::vector<const char *> flags = IMAPResponse::buildMailboxFlags(mailbox.e_Flags);
			result += IMAPResponse::build({
				BuildLine {
					nullptr,
					{
						BuildLineSection {
							BVT_COMMAND, reinterpret_cast<const void *>(&type)
						},
						BuildLineSection {
							BVT_LIST, reinterpret_cast<const void *>(&flags)
						},
						BuildLineSection {
							BVT_STRING, reinterpret_cast<const void *>(".")
						},
						BuildLineSection {
							BVT_STRING, reinterpret_cast<const void *>(mailbox.e_MailboxPath.c_str())
						}
					}
				}
			});
		}

		// Builds the completed message and returns
		result += IMAPResponse::build({
			BuildLine {
				tagIndex.c_str(),
				{
					BuildLineSection {
						BVT_OK, nullptr
					},
					BuildLineSection {
						BVT_COMPLETED, reinterpret_cast<const void *>(&type)
					}
				}
			}
		});
		return result;
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
		return IMAPResponse::build({
			BuildLine {
				tagIndex.c_str(),
				{
					BuildLineSection {
						BVT_OK, nullptr
					},
					BuildLineSection {
						BVT_COMPLETED, reinterpret_cast<const void *>(&type)
					}
				}
			}
		});
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
			case IMAPCommandType::ICT_NOOP: return "NOOP";
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
		return IMAPResponse::build({
			BuildLine {
				nullptr,
				{
					BuildLineSection {
						BVT_OK, nullptr
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>(_SMTP_SERVICE_NODE_NAME)
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>("Fannst IMAP4rev1 service ready - max 1.800.000ms")
					}
				}
			}
		});
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
		return IMAPResponse::build({
			BuildLine {
				index.c_str(),
				{
					BuildLineSection {
						BVT_BAD, nullptr
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>(reason.c_str())
					}
				}
			}
		});
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
		return IMAPResponse::build({
			BuildLine {
				index.c_str(),
				{
					BuildLineSection {
						BVT_NO, nullptr
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>(reason.c_str())
					}
				}
			}
		});
	}

	/**
	 * Builds the select information header, when an mailbox is selected
	 *
	 * @Param {const std::string &} index
	 * @Param {const MailboxStatys &} status
	 * @Return {std::string}
	 */
	std::string IMAPResponse::buildSelectInformation(
		const std::string &index,
		const MailboxStatus &status
	)
	{
		static IMAPCommandType type = IMAPCommandType::ICT_SELECT;
		char uidValidity[64];
		char uidNext[64];
		char unseenStr[64];

		// Prints to the buffers
		sprintf(uidValidity, "%s %d", "UIDVALIDITY", std::numeric_limits<int32_t>::max());
		sprintf(unseenStr, "%s %d", "UNSEEN", status.s_Unseen);
		sprintf(uidNext, "%s %d", "UIDNEXT", status.s_NextUID);

		// Builds the flags
		std::vector<const char *> flags = IMAPResponse::buildMailboxFlags(status.s_Flags);

		// Builds the result
		return IMAPResponse::build({
			BuildLine {
				nullptr,
				{
					BuildLineSection {
						BVT_INT32, reinterpret_cast<const void *>(&status.s_Total)
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>("EXISTS")
					}
				}
			},
			BuildLine {
				nullptr,
				{
					BuildLineSection {
						BVT_INT32, reinterpret_cast<const void *>(&status.s_Recent)
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>("RECENT")
					}
				}
			},
			BuildLine {
				nullptr,
				{
					BuildLineSection{IMAP::BVT_OK, nullptr},
					BuildLineSection{
						BVT_BRACKETS, 
						reinterpret_cast<const void *>(unseenStr)
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>("Message")
					},
					BuildLineSection {
						BVT_INT32, reinterpret_cast<const void *>(&status.s_Unseen)
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>("is first unseen")
					},
				}
			},
			BuildLine {
				nullptr,
				{
					BuildLineSection{IMAP::BVT_OK, nullptr},
					BuildLineSection{
						BVT_BRACKETS, reinterpret_cast<const void *>(uidValidity)
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>("UIDs valid")
					}
				}
			},
			BuildLine {
				nullptr,
				{
					BuildLineSection{IMAP::BVT_OK, nullptr},
					BuildLineSection{
						BVT_BRACKETS, reinterpret_cast<const void *>(uidNext)
					},
					BuildLineSection {
						BVT_ATOM, reinterpret_cast<const void *>("predicted next UID")
					}
				}
			},
			BuildLine {
				nullptr,
				{
					BuildLineSection{
						BVT_ATOM, reinterpret_cast<const void *>("FLAGS")
					},
					BuildLineSection {
						BVT_LIST, reinterpret_cast<const void *>(&flags)
					}
				}
			},
			BuildLine {
				nullptr,
				{
					BuildLineSection{IMAP::BVT_OK, nullptr},
					BuildLineSection{
						BVT_BRACKETS, 
						reinterpret_cast<const void *>(
							"PERMANENTFLAGS (\\Flagged \\Seen \\Answered \\Deleted \\Draft)")
					},
					BuildLineSection{
						BVT_ATOM, reinterpret_cast<const void *>("LIMITED")
					},
				}
			},
			BuildLine {
				"JZT1",
				{
					BuildLineSection{IMAP::BVT_OK, nullptr},
					BuildLineSection{
						BVT_BRACKETS, reinterpret_cast<const void *>("READ-WRITE")
					},
					BuildLineSection {
						BVT_COMPLETED, reinterpret_cast<const void *>(&type)
					}
				}
			}
		});
	}

	/**
	 * Builds an list of email flags
	 *
	 * @Param {const int32_t} flags
	 * @return {std::string}
	 */
	std::string IMAPResponse::buildEmailFlagList(const int32_t flags)
	{
		std::vector<const char *> vec = {};
		if (BINARY_COMPARE(flags, _EMAIL_FLAG_RECENT))
			vec.push_back("\\RECENT");
		if (BINARY_COMPARE(flags, _EMAIL_FLAG_SEEN))
			vec.push_back("\\SEEN");
		if (BINARY_COMPARE(flags, _EMAIL_FLAG_DELETED))
			vec.push_back("\\DELETED");
		if (BINARY_COMPARE(flags, _EMAIL_FLAG_ANSWERED))
			vec.push_back("\\ANSWERERED");
		if (BINARY_COMPARE(flags, _EMAIL_FLAG_DRAFT))
			vec.push_back("\\DRAFT");

		std::string res = "(";
		std::size_t i = 0;
		for (const char *p : vec)
		{
			res += p;
			if (++i < vec.size()) res += ' ';
		}
		res += ')';
		return res;
	}

	/**
	 * Builds an response message
	 *
	 * @Param {const std::vector<BuildLine> &} lines
	 */
	std::string IMAPResponse::build(const std::vector<BuildLine> &lines)
	{
		std::ostringstream os;
		for (auto &line : lines)
		{
			// Adds the section start, either the index
			// - or the default "* "
			if (line.b_Index == nullptr) os << "* ";
			else os << line.b_Index << ' ';

			// Loops and appends the sections
			std::size_t i = 0;
			for (auto &section : line.b_Sections)
			{
				// Checks how to insert the value
				switch (section.b_Type)
				{
					case BuildLineSectionType::BVT_LIST:
					{
						// Checks if it is not an nullptr
						if (section.b_Value == nullptr)
							throw std::invalid_argument(EXCEPT_DEBUG("[BVT_LIST] Value may not be nullptr"));

						// Appends the list elements
						os << '(';
						std::size_t listIndex = 0;
						auto &vec = *reinterpret_cast<const std::vector<const char *> *>(section.b_Value);
						for (const char *p : vec)
						{
							os << p;
							if (++listIndex < vec.size()) os << ' ';
						}
						os << ')';
						break;
					}
					case BuildLineSectionType::BVT_STRING:
					{
						// Checks if it is not an nullptr
						if (section.b_Value == nullptr)
							throw std::invalid_argument(EXCEPT_DEBUG("[BVT_STRING] Value may not be nullptr"));

						os << '"' << reinterpret_cast<const char *>(section.b_Value) << '"';
						break;
					}
					case BuildLineSectionType::BVT_ATOM:
					{
						// Checks if it is not an nullptr
						if (section.b_Value == nullptr)
							throw std::invalid_argument(EXCEPT_DEBUG("[BVT_ATOM] Value may not be nullptr"));

						os << reinterpret_cast<const char *>(section.b_Value);
						break;
					}
					case BuildLineSectionType::BVT_BRACKETS:
					{
						// Checks if it is not an nullptr
						if (section.b_Value == nullptr)
							throw std::invalid_argument(EXCEPT_DEBUG("[BVT_BRACKETS] Value may not be nullptr"));

						os << '[' << reinterpret_cast<const char *>(section.b_Value) << ']';
						break;
					}
					case BuildLineSectionType::BVT_OK:
					{
						os << "OK";
						break;
					}
					case BuildLineSectionType::BVT_BAD:
					{
						os << "BAD";
						break;
					}
					case BuildLineSectionType::BVT_NO:
					{
						os << "NO";
						break;
					}
					case BuildLineSectionType::BVT_COMMAND:
					{
							// Checks if it is not an nullptr
						if (section.b_Value == nullptr)
							throw std::invalid_argument(EXCEPT_DEBUG("[BVT_COMMAND] Value may not be nullptr"));

						os << IMAPResponse::getCommandStr(
							*reinterpret_cast<const IMAPCommandType *>(section.b_Value));
						break;
					}
					case BuildLineSectionType::BVT_COMPLETED:
					{
						// Checks if it is not an nullptr
						if (section.b_Value == nullptr)
							throw std::invalid_argument(EXCEPT_DEBUG("[BVT_COMPLETED] Value may not be nullptr"));

						// Appends the completed command
						os << IMAPResponse::getCommandStr(
							*reinterpret_cast<const IMAPCommandType *>(section.b_Value))
							<< " completed";
						break;
					}
					case BuildLineSectionType::BVT_INT32:
					{
						// Checks if it is not an nullptr
						if (section.b_Value == nullptr)
							throw std::invalid_argument(EXCEPT_DEBUG("[BVT_NUMBER] Value may not be nullptr"));

						os << *reinterpret_cast<const int32_t *>(section.b_Value);
						break;
					}
					default: throw std::runtime_error(EXCEPT_DEBUG("Type not implemented !"));
				}

				// Checks if we need to insert whitespace, if so
				// - insert it
				if (++i < line.b_Sections.size()) os << ' ';
			}
			os << "\r\n";
		}
		return os.str();
	}
}
