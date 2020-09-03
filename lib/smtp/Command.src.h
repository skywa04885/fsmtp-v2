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

#include "../default.h"
#include "../general/cleanup.src.h"

using namespace FSMTP::Cleanup;

namespace FSMTP::SMTP
{
	typedef enum : uint32_t {
		CCT_HELO = 0,
		CCT_EHLO,
		CCT_START_TLS,
		CCT_MAIL_FROM,
		CCT_RCPT_TO,
		CCT_DATA,
		CCT_QUIT,
		CCT_UNKNOWN,
		CCT_AUTH,
		CCT_HELP,
		CCT_SU
	} ClientCommandType;

	class ClientCommand {
	public:
		ClientCommand(const ClientCommandType c_CommandType, const vector<string> &c_Arguments);
		ClientCommand();
		ClientCommand(const string &raw);
		string build(void);
		
		ClientCommandType c_CommandType;
		vector<string> c_Arguments;
	};
}
