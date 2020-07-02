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

#include <iostream>
#include <cstdint>

#define _SMTP_SERV_SESSION_RELAY_FLAG 1
#define _SMTP_SERV_SESSION_AUTH_FLAG 2
#define _SMTP_SERV_SESSION_SSL_FLAG 4

#define _SMTP_SERV_PA_HELO 1
#define _SMTP_SERV_PA_START_TLS 2
#define _SMTP_SERV_PA_MAIL_FROM 4
#define _SMTP_SERV_PA_RCPT_TO 8
#define _SMTP_SERV_PA_DATA_START 16
#define _SMTP_SERV_PA_DATA_END 32
#define _SMTP_SERV_PA_HELO_AFTER_STLS 64

namespace FSMTP::Server
{
	class SMTPServerSession
	{
	public:

	private:
		int32_t s_Flags;
		int64_t s_PerformedActions;
	};
}