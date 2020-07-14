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
	 * @Param {const std::vector<SMTPServiceFunction *} c_Services
	 * @Return {void}
	 */
	ServerResponse::ServerResponse(
		const SMTPResponseType c_Type,
		const std::string &c_Message, 
		std::vector<SMTPServiceFunction> *c_Services
	):
		c_Type(c_Type), c_Services(c_Services), c_Message(c_Message)
	{}

	/**
	 * Builds the response message
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string build(void)
	{
		std::string res;

		return res;
	}

	/**
	 * Gets the message for an specific response type
	 *
	 * @Param {const SMTPResponseType} c_Type
	 * @Return {const char *}
	 */
	const char *ServerResponse::getMessage(const SMTPResponseType c_Type)
	{

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
			default: return 999;
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
			clean.substr(++index)
		);
	}
}