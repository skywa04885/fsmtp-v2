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

#include "../general/macros.src.h"

namespace FSMTP::SMTP
{
	typedef enum : uint32_t {
		SRC_INIT = 0,
		SRC_HELO_RESP,
		SRC_READY_START_TLS,
		SRC_PROCEED,
		SRC_QUIT_RESP,
		SRC_SYNTAX_ARG_ERR,
		SRC_SYNTAX_ERR_INVALID_COMMAND,
		SRC_BAD_EMAIL_ADDRESS,
		SRC_DATA_START,
		SRC_DATA_END
	} SMTPResponseCommand;

	typedef struct {
		const char *s_Name;
		const std::vector<const char *> s_SubArgs;
	} SMTPServiceFunction;

	class ServerResponse
	{
	public:
		/**
		 * Default constructor which will throw default responses
		 *
		 * @Param {SMTPResponseCommand &} r_CType
		 * @Param {bool &} r_ESMTP
		 * @Param {std::vector<SMTPServiceFunction> *} services
		 * @Return void
		 */
		ServerResponse(
			const SMTPResponseCommand &r_CType,
			const bool &r_ESMTP, 
			const std::vector<SMTPServiceFunction> *services
		);

		/**
		 * The default constructor for an custom user response
		 * - but we will still add the status code by default
		 *
		 * @Param {SMTPResponseCommand &} r_CType
		 * @Param {std::string &} r_Message
		 * @Return void
		 */
		ServerResponse(
			const SMTPResponseCommand &r_CType, 
			const std::string &r_Message
		);

		/**
		 * Builds the response and stores it in the string reference
		 *
		 * @Param {std::string &} ret
		 * @Return void
		 */
		void build(std::string &ret);

		/**
		 * Static method which turns an enum value
		 * - into an usable code in the string format
		 *
		 * @Param {SMTPResponseCommand &} c
		 * @Return const char *
		 */
		static const char *rcToCode(const SMTPResponseCommand &c);
	private:
		std::string r_Message;
		SMTPResponseCommand r_CType;
		bool r_ESMTP;
	};
}