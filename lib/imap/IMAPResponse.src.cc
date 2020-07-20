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
	 * Default constructor for the imap response
	 *
	 * @Param {const IMAPResponseType} r_Type
	 * @Param {const std::string} r_TagIndex
	 * @Param {const IMAPResponseStructure} r_Structure
	 * @Param {const IMAPResponsePrefixType} r_PrefType
	 * @Param {void *} r_U
	 * @Return {void}
	 */
	IMAPResponse::IMAPResponse(
		const IMAPResponseStructure r_Structure,
		const std::string r_TagIndex,
		const IMAPResponseType r_Type,
		const IMAPResponsePrefixType r_PrefType,
		void *r_U
	):
		r_Type(r_Type), r_Structure(r_Structure), r_TagIndex(r_TagIndex),
		r_PrefType(r_PrefType), r_U(r_U)
	{}

	/**
	 * Default constructor for the imap response
	 *
	 * @Param {const IMAPResponseType} r_Type
	 * @Param {const std::string} r_TagIndex
	 * @Param {const IMAPResponseStructure} r_Structure
	 * @Param {const IMAPResponsePrefixType} r_PrefType
	 * @Param {void *} r_U
	 * @Return {void}
	 */
	IMAPResponse::IMAPResponse(
		const IMAPResponseStructure r_Structure,
		const std::string r_TagIndex,
		const IMAPResponseType r_Type,
		const IMAPResponsePrefixType r_PrefType,
		const std::string &r_Message,
		void *r_U
	):
		r_Type(r_Type), r_Structure(r_Structure), r_TagIndex(r_TagIndex),
		r_PrefType(r_PrefType), r_U(r_U), r_Message(r_Message)
	{}

	/**
	 * Builds the response
	 *
	 * @Param {const IMAPResponseType} r_Type
	 * @Return {void}
	 */
	std::string IMAPResponse::build(void)
	{
		switch (this->r_Structure)
		{
			case IRS_NTATL:
			{
				std::string ret = "* ";

				// Adds the message
				ret += this->getMessage();
				ret += "\r\n";
				ret += this->r_TagIndex;
				ret += ' ';

				// Adds the command name
				ret += this->getPrefix();
				ret += this->getCommandName();
				ret += " completed\r\n";

				return ret;
			}
			case IRS_NT:
			{
				std::string ret = "* ";

				// Adds the prefix and message
				ret += this->getPrefix();
				ret += this->getMessage();
				ret += "\r\n";

				return ret;
			}
			case IRS_TL:
			{
				std::string ret = this->r_TagIndex;
				ret += ' ';
				ret += this->getPrefix();
				ret += this->getMessage();
				ret += "\r\n";

				return ret;
			}
			case IRS_TLC:
			{
				std::string ret = this->r_TagIndex;
				ret += ' ';

				// Adds the command name
				ret += this->getPrefix();
				ret += this->getCommandName();
				ret += " completed\r\n";

				return ret;
			}
			default: throw std::runtime_error(EXCEPT_DEBUG("Structure not implemented"));
		}
	}

	/**
	 * Gets an command prefix
	 *
	 * @Param {void}
	 * @Return {const char *} 
	 */
	const char *IMAPResponse::getPrefix(void)
	{
		switch (this->r_PrefType)
		{
			case IMAPResponsePrefixType::IPT_BAD: return "BAD ";
			case IMAPResponsePrefixType::IPT_OK: return "OK ";
			case IMAPResponsePrefixType::IPT_NO: return "NO ";
			case IMAPResponsePrefixType::IPT_BYE: return "BYE ";
			default: throw std::runtime_error(EXCEPT_DEBUG("Not implemented"));
		}
	}


	/**
	 * Gets the name of the command type
	 *
	 * @Param {void}
	 * @Return {const char *}
	 */
	const char *IMAPResponse::getCommandName(void)
	{
		switch (this->r_Type)
		{
			case IMAPResponseType::IRT_GREETING: return "GREETING";
			case IMAPResponseType::IRT_LOGOUT: return "LOGOUT";
			case IMAPResponseType::IRT_CAPABILITIES: return "CAPABILITY";
			case IMAPResponseType::IRT_LOGIN_SUCCESS: return "LOGIN";
			case IMAPResponseType::IRT_STARTTLS: return "STARTTLS";
			default: throw std::runtime_error(EXCEPT_DEBUG("Not implemented"));
		}
	}

	/**
	 * Gets the message
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string IMAPResponse::getMessage(void)
	{
		if (!this->r_Message.empty()) return this->r_Message;
		switch (this->r_Type)
		{
			case IMAPResponseType::IRT_GREETING:
			{
				char dateBuffer[128];
				std::time_t rawTime;
				struct tm *timeInfo = nullptr;

				// Builds the standard message
				std::string ret = _SMTP_SERVICE_NODE_NAME;
				ret += " Fannst IMAP4rev1 service ready at, ";

				// Appends the date
				time(&rawTime);
				timeInfo = localtime(&rawTime);
				strftime(
					dateBuffer,
					sizeof (dateBuffer),
					"%a, %d %b %Y %T %Z",
					timeInfo
				);
				ret += dateBuffer;

				return ret;
			}
			case IRT_LOGOUT: return "Fannst IMAP4rev1 server terminating connection";
			case IRT_CAPABILITIES: return IMAPResponse::buildCapabilities(
				*reinterpret_cast<std::vector<IMAPCapability> *>(this->r_U)
			);
			default: throw std::runtime_error(EXCEPT_DEBUG("Command not implemented"));
		}
	}

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
}
