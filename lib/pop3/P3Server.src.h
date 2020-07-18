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
#include "P3ServerSocket.src.h"
#include "P3Commands.src.h"
#include "P3Response.src.h"
#include "P3ServerSession.src.h"

using namespace FSMTP::Connections;
using namespace FSMTP::Models;

namespace FSMTP::POP3
{
	class P3Server
	{
	public:
		P3Server(const bool secure);

		static void acceptorCallback(std::unique_ptr<ClientSocket> client, void *u);

		/**
		 * Stops the pop3 server
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void shutdown(void);
	private:
		ServerSocket s_Socket;
		Logger s_Logger;
		std::atomic<bool> s_Run;
		std::atomic<bool> s_Running;
		std::vector<POP3Capability> s_Capabilities;
	};
}
