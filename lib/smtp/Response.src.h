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
		SRC_SYNTAX_ERR_INVALID_COMMAND
	} SMTPResponseCommand;

	typedef struct {
		const char *s_Name;
		const std::vector<const char *> s_SubArgs;
	} SMTPServiceFunction;

	class ServerResponse
	{
	public:
		ServerResponse(
			const SMTPResponseCommand &r_CType,
			const bool &r_ESMTP, 
			const std::vector<SMTPServiceFunction> *services
		);
		ServerResponse(const SMTPResponseCommand &r_CType, const std::string &r_Message);

		void build(std::string &ret);
	private:
		std::string r_Message;
		SMTPResponseCommand r_CType;
		bool r_ESMTP;
	};
}