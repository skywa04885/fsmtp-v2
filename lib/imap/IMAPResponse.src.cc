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
	 * @Param {const int32_t} r_TagIndex
	 * @Param {const bool} r_Untagged
	 * @Param {const IMAPResponsePrefixType} r_PrefType
	 * @Param {void *} r_U
	 * @Return {void}
	 */
	IMAPResponse::IMAPResponse(
		const bool r_Untagged,
		const int32_t r_TagIndex,
		const IMAPResponseType r_Type,
		const IMAPResponsePrefixType r_PrefType,
		void *r_U
	):
		r_Type(r_Type), r_Untagged(r_Untagged), r_TagIndex(r_TagIndex),
		r_PrefType(r_PrefType), r_U(r_U)
	{}

	/**
	 * Builds the response
	 *
	 * @Param {const IMAPResponseType} r_Type
	 * @Return {void}
	 */
	std::string IMAPResponse::build(void)
	{
		std::string ret;

		// Appends the tag, or taggles char and after that
		// - we will add the prefix
		if (this->r_Untagged) ret += "* ";
		else
		{
			ret += std::to_string(this->r_TagIndex);
		}

		switch (this->r_PrefType)
		{
			case IMAPResponsePrefixType::IPT_BAD:
			{
				ret += "BAD ";
				break;
			}
			case IMAPResponsePrefixType::IPT_OK:
			{
				ret += "OK ";
				break;
			}
			case IMAPResponsePrefixType::IPT_NO:
			{
				ret += "NO ";
				break;
			}
		}

		// Adds the message and returns
		ret += this->getMessage();
		ret += "\r\n";
		return ret;
	}

	/**
	 * Gets the message
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string IMAPResponse::getMessage(void)
	{
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
			case IRT_CAPABILITIES: return IMAPResponse::buildCapabilities(
				reinterpret_cast<std::vector<IMAPCapability> &capabilities>(this->r_U)
			);
			default: throw std::runtime_error(EXCEPT_DEBUG("Command not implemented"));
		}
	}

	std::string IMAPResponse::buildCapabilities(
		const std::vector<IMAPCapability> &capabilities
	)
	{
		std::string ret;
		for_each(capabilities.begin(), capabilities.end(), [=](const IMAPCapability &c){
			ret += c.c_Key;
			if (c.c_Value != nullptr)
			{
				ret += '=';
				ret += c.c_Value;
			}
		});
		return ret;
	}
}
