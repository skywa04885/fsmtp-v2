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

#include "P3Response.src.h"

namespace FSMTP::POP3
{
	P3Response::P3Response(const bool p_Ok, const POP3ResponseType p_Type):
		p_Type(p_Type), p_Ok(p_Ok), p_Capabilities(nullptr)
	{}

	std::string P3Response::build(void)
	{
		std::string res;

		// Adds ok or failure, and then the message
		if (!p_Ok) res += "-ERR ";
		else res += "+OK ";
		res += this->getMessage();

		// Checks if we need to append capabilities
		if (this->p_Capabilities != nullptr)
		{
			res += "\r\n";
			res += buildCapabilities();
		} else res += "\r\n";

		return res;
	}

	std::string P3Response::getMessage(void)
	{
		switch (this->p_Type)
		{
			case POP3ResponseType::PRT_GREETING:
			{
				std::string ret = "POP3 server ready ";
				ret += _SMTP_SERVICE_NODE_NAME;
				return ret;
			}
			case POP3ResponseType::PRT_CAPA:
			{
				return "Capability list follows";
			}
			case POP3ResponseType::PRT_STLS_START:
			{
				return "Begin TLS negotiation now";
			}
			case POP3ResponseType::PRT_COMMAND_INVALID:
			{
				return "Invalid command";
			}
			case POP3ResponseType::PRT_USER_DONE:
			{
				return "Send PASS";
			}
			case POP3ResponseType::PRT_AUTH_SUCCESS:
			{
				return "Auth Success";
			}
			case POP3ResponseType::PRT_AUTH_FAIL:
			{
				std::string ret = "Auth Failure: ";
				ret += reinterpret_cast<const char *>(this->p_U);
				return ret;
			}
			case POP3ResponseType::PRT_SYNTAX_ERROR:
			{
				std::string ret = "Syntax Error: ";
				ret += reinterpret_cast<const char *>(this->p_U);
				return ret;
			}
			case POP3ResponseType::PRT_QUIT:
			{
				std::string ret = "POP3 server signing off ";
				ret += _SMTP_SERVICE_NODE_NAME;
				return ret;
			}
			default: throw std::runtime_error(EXCEPT_DEBUG("Message not implemented"));
		}
	}

	/**
	 * Builds the list of capabilities
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string P3Response::buildCapabilities(void)
	{
		std::string res;

		for (const POP3Capability &cap : *this->p_Capabilities)
		{
			// Adds the name
			res += cap.c_Name;

			// Adds the subargs
			for (const char *arg : cap.c_Args)
			{
				res += ' ';
				res += arg;
			}

			// Adds the newline
			res += "\r\n";
		}

		// Adds the dot
		res += ".\r\n";
		return res;
	}

	P3Response::P3Response(
		const bool p_Ok,
		const POP3ResponseType p_Type,
		const std::string &p_Message,
		std::vector<POP3Capability> *p_Capabilities,
		void *p_U
	):
		p_Ok(p_Ok), p_Type(p_Type), p_Message(p_Message),
		p_Capabilities(p_Capabilities), p_U(p_U)
	{}
}
