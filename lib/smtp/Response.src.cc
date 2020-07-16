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

#include "Response.src.h"

namespace FSMTP::SMTP
{
	/**
	 * The default constructor for the response
	 * - this will generate the text, and do everything
	 * - automatically
	 *
	 * @Param {const SMTPResponseType} c_Type
	 * @Return {void}
	 */
	ServerResponse::ServerResponse(const SMTPResponseType c_Type):
		c_Type(c_Type), c_Services(nullptr)
	{}

	/**
	 * The default constructor, but then with the services
	 * - pointer, currently only for EHLO command
	 *
	 * @Param {const SMTPResponseType} c_Type
	 * @Param {const std::string &} c_Message
	 * @Param {void *} c_U
	 * @Param {const std::vector<SMTPServiceFunction *} c_Services
	 * @Return {void}
	 */
	ServerResponse::ServerResponse(
		const SMTPResponseType c_Type,
		const std::string &c_Message, 
		void *c_U,
		std::vector<SMTPServiceFunction> *c_Services
	):
		c_Type(c_Type), c_Services(c_Services), c_Message(c_Message),
		c_U(c_U)
	{}

	/**
	 * Builds the response message
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string ServerResponse::build(void)
	{
		std::string res;
		int32_t code = ServerResponse::getCode(this->c_Type);

		// Checks if we need to generate
		// - an normal message
		if (this->c_Services == nullptr)
		{
			res += std::to_string(code);
			res += ' ';
			res += this->getMessage(this->c_Type);

			if (this->c_Type != SMTPResponseType::SRC_GREETING)
			{
				res += ' ';
				res += _SMTP_SERVICE_NODE_NAME;
			}
			res += " - fsmtp\r\n";
		} else
		{
			res += std::to_string(code);
			res += '-';
			res += this->getMessage(this->c_Type);
			res += "\r\n";
			res += ServerResponse::buildServices(code, this->c_Services);
		}

		return res;
	}

	/**
	 * Gets the message for an specific response type
	 *
	 * @Param {const SMTPResponseType} c_Type
	 * @Return {std::string}
	 */
	std::string ServerResponse::getMessage(const SMTPResponseType c_Type)
	{
		if (!this->c_Message.empty()) return this->c_Message;

		switch (c_Type)
		{
			case SMTPResponseType::SRC_GREETING:
			{
				char dateBuffer[128];
				std::time_t rawTime;
				struct tm *timeInfo = nullptr;

				// Builds the standard message
				std::string ret = _SMTP_SERVICE_NODE_NAME;
				ret += " Fannst ESMTP Mail service ready at ";

				// Appends the time to the final string
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
			case SMTPResponseType::SRC_HELO:
			case SMTPResponseType::SRC_EHLO:
			{
				// Builds the response message and returns it
				std::string ret = _SMTP_SERVICE_DOMAIN;
				ret += ", at your service ";
				ret += DNS::getHostnameByAddress(reinterpret_cast<struct sockaddr_in *>(this->c_U));
				ret += " [";
				ret += inet_ntoa(reinterpret_cast<struct sockaddr_in *>(this->c_U)->sin_addr);
				ret += ']';
				return ret;
			}
			case SMTPResponseType::SRC_MAIL_FROM:
			case SMTPResponseType::SRC_RCPT_TO:
			{
				std::string ret = "OK, proceed [";
				ret += reinterpret_cast<const char *>(this->c_U);
				ret += ']';
				return ret;;
			}
			case SMTPResponseType::SRC_DATA_START:
			{
				return "End data with <CR><LF>.<CR><LF>";
			}
			case SMTPResponseType::SRC_ORDER_ERR:
			{
				return "Invalid order, why: [unknown].";
			}
			case SMTPResponseType::SRC_INVALID_COMMAND:
			{
				return "unrecognized command.";
			}
			case SMTPResponseType::SRC_START_TLS:
			{
				return "Ready to start TLS.";
			}
			case SMTPResponseType::SRC_QUIT_GOODBYE:
			{
				return "closing connection";
			}
			case SMTPResponseType::SRC_AUTH_SUCCESS:
			{
				return "Authentication successfull, welcome back.";
			}
			case SMTPResponseType::SRC_AUTH_FAIL:
			{
				return "Authentication failed, closing transmission channel.";
			}
			case SMTPResponseType::SRC_REC_NOT_LOCAL:
			{
				std::string ret = "User [";
				ret += reinterpret_cast<const char *>(this->c_U);
				ret += "] not local, closing transmission channel.";
				return ret;
			}
			case SMTPResponseType::SRC_DATA_END:
			{
				std::string ret = "OK, message queued ";
				ret += reinterpret_cast<const char *>(this->c_U); // ( MESSAGE ID )
				return ret;
			}
			default: throw std::runtime_error("getMessage() invalid type");
		}
	}

	/**
	 * Gets the code for response type
	 *
	 * @Param {const SMTPResponseType} c_Type
	 * @Return {int32_t}
	 */
	int32_t ServerResponse::getCode(const SMTPResponseType c_Type)
	{
		switch (c_Type)
		{
			case SMTPResponseType::SRC_GREETING: return 220;
			case SMTPResponseType::SRC_EHLO: return 250;
			case SMTPResponseType::SRC_HELO: return 250;
			case SMTPResponseType::SRC_MAIL_FROM: return 250;
			case SMTPResponseType::SRC_RCPT_TO: return 250;
			case SMTPResponseType::SRC_DATA_START: return 354;
			case SMTPResponseType::SRC_DATA_END: return 250;
			case SMTPResponseType::SRC_QUIT_GOODBYE: return 221;
			case SMTPResponseType::SRC_SYNTAX_ERR: return 501;
			case SMTPResponseType::SRC_ORDER_ERR: return 503;
			case SMTPResponseType::SRC_INVALID_COMMAND: return 502;
			case SMTPResponseType::SRC_START_TLS: return 220;
			case SMTPResponseType::SRC_REC_NOT_LOCAL: return 551;
			case SMTPResponseType::SRC_AUTH_SUCCESS: return 235;
			case SMTPResponseType::SRC_AUTH_FAIL: return 530;
			default: throw std::runtime_error("getCode() invalid type");
		}
	}

	/**
	 * Builds the services list
	 *
	 * @Param {const int32_t} code
	 * @Param {std::vector<SMTPServiceFunction> *} c_Services
	 */
	std::string ServerResponse::buildServices(
		const int32_t code,
		std::vector<SMTPServiceFunction> *c_Services
	)
	{
		std::string res;

		std::size_t index = 0, total = c_Services->size();
		for (const SMTPServiceFunction service  : *c_Services)
		{
			// Appends the code with the
			// - required separator and name
			res += std::to_string(code);
			if (++index == total)
				res += ' ';
			else
				res += '-';
			res += service.s_Name;

			// Appends the space with the sub arguments
			// - to the result
			for (const char *arg : service.s_SubArgs)
			{
				res += ' ';
				res += arg;
			}

			// Appends the newline
			res += "\r\n";
		}

		return res;
	}

	/**
	 * Parses an server response into an string and code
	 *
	 * @Param {const std::string &} raw
	 * @Return {int32_t}
	 * @Return {std::string}
	 */
	std::tuple<int32_t, std::string> ServerResponse::parseResponse(const std::string &raw)
	{
		std::string clean;
		reduceWhitespace(raw, clean);

		std::size_t index = clean.find_first_of(' ');
		if (index == std::string::npos)
			return std::make_pair(std::stoi(clean), "");
		else return std::make_pair(
			std::stoi(clean.substr(0, index)),
			clean.substr(index + 1)
		);
	}
}