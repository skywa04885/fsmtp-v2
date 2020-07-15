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

#include <vector>
#include <string>
#include <cstdint>
#include <tuple>
#include <ctime>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "../general/macros.src.h"
#include "../general/cleanup.src.h"
#include "../networking/DNS.src.h"

using namespace FSMTP::Cleanup;

namespace FSMTP::SMTP
{
	typedef enum : uint32_t {
		SRC_GREETING,
		SRC_EHLO,
		SRC_HELO,
		SRC_MAIL_FROM,
		SRC_RCPT_TO,
		SRC_DATA_START,
		SRC_DATA_END,
		SRC_QUIT_GOODBYE,
		SRC_SYNTAX_ERR,
		SRC_ORDER_ERR,
		SRC_INVALID_COMMAND,
		SRC_START_TLS
	} SMTPResponseType;

	typedef struct {
		const char *s_Name;
		const std::vector<const char *> s_SubArgs;
	} SMTPServiceFunction;

	class ServerResponse
	{
	public:
		/**
		 * The default constructor for the response
		 * - this will generate the text, and do everything
		 * - automatically
		 *
		 * @Param {const SMTPResponseType} c_Type
		 * @Return {void}
		 */
		ServerResponse(const SMTPResponseType c_Type);

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
		ServerResponse(
			const SMTPResponseType c_Type,
			const std::string &c_Message,
			void *c_U,
			std::vector<SMTPServiceFunction> *c_Services
		);

		/**
		 * Builds the response message
		 *
		 * @Param {void}
		 * @Return {std::string}
		 */
		std::string build(void);

		/**
		 * Gets the message for an specific response type
		 *
		 * @Param {const SMTPResponseType} c_Type
		 * @Return {std::string}
		 */
		std::string getMessage(const SMTPResponseType c_Type);

		/**
		 * Gets the code for response type
		 *
		 * @Param {const SMTPResponseType} c_Type
		 * @Return {int32_t}
		 */
		static int32_t getCode(const SMTPResponseType c_Type);

		/**
		 * Builds the services list
		 *
		 * @Param {const int32_t} code
		 * @Param {std::vector<SMTPServiceFunction> *} c_Services
		 */
		static std::string buildServices(
			const int32_t code,
			std::vector<SMTPServiceFunction> *c_Services
		);

		/**
		 * Parses an server response into an string and code
		 *
		 * @Param {const std::string &} raw
		 * @Return {int32_t}
		 * @Return {std::string}
		 */
		static std::tuple<int32_t, std::string> parseResponse(const std::string &raw);
	private:
		SMTPResponseType c_Type;
		std::vector<SMTPServiceFunction> *c_Services;
		std::string c_Message;
		void *c_U;
	};
}