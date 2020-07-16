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

#include "P3.src.h"
#include "P3Response.src.h"

namespace FSMTP::POP3
{
	class ClientSocket
	{
	public:
		ClientSocket(const int32_t fd, const struct sockaddr_in client);

		~ClientSocket(void);

		std::string readUntillCRLF(void);

		void sendString(const std::string &raw);
		void sendResponse(const bool p_Ok, const POP3ResponseType p_Type);
	private:
		struct sockaddr_in s_SocketAddr;
		int32_t s_SocketFD;
		Logger s_Logger;
	};
}