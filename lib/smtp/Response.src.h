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
#include "../general/Global.src.h"

#include "../general/cleanup.src.h"
#include "../networking/DNS.src.h"

using namespace FSMTP::Cleanup;

namespace FSMTP::SMTP
{
	typedef enum : uint32_t {
		SRC_GREETING = 0,
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
		SRC_START_TLS,
		SRC_REC_NOT_LOCAL,
		SRC_AUTH_SUCCESS,
		SRC_AUTH_FAIL,
		SRC_RELAY_FAIL,
		SRC_HELP_RESP,
		SRC_AUTH_NOT_ALLOWED,
		SRC_SPAM_ALERT
	} SMTPResponseType;

	typedef struct {
		const char *s_Name;
		const std::vector<const char *> s_SubArgs;
	} SMTPServiceFunction;

	class ServerResponse
	{
	public:
		ServerResponse(const SMTPResponseType c_Type);
		ServerResponse(
			const SMTPResponseType c_Type, const string &c_Message,
			const void *c_U, vector<SMTPServiceFunction> *c_Services
		);

		string build(void);
		string getMessage(const SMTPResponseType c_Type);

		static int32_t getCode(const SMTPResponseType c_Type);
		static string buildServices(const int32_t code, vector<SMTPServiceFunction> *c_Services);
		static tuple<int32_t, string> parseResponse(const std::string &raw);
	private:
		SMTPResponseType c_Type;
		vector<SMTPServiceFunction> *c_Services;
		string c_Message;
		const void *c_U;
	};
}