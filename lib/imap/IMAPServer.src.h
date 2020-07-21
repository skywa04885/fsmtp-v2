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

#include "IMAP.src.h"
#include "IMAPServerSocket.src.h"
#include "IMAPResponse.src.h"
#include "IMAPCommand.src.h"
#include "IMAPAuthHandler.src.h"
#include "IMAPServerSession.src.h"
#include "IMAPMessageHandler.src.h"
#include "IMAPMailboxHandler.src.h"

namespace FSMTP::IMAP
{
	class IMAPServer
	{
	public:
		IMAPServer(const int32_t plainPort, const int32_t securePort);

		static void acceptorCallback(
			std::unique_ptr<IMAPClientSocket> client,
			void *u
		);

		std::vector<IMAPCapability> s_SecureCapabilities;
		std::vector<IMAPCapability> s_PlainCapabilities;
	private:
		IMAPServerSocket s_Socket;
		std::atomic<bool> s_SecureRunning;
		std::atomic<bool> s_PlainRunning;
		std::atomic<bool> s_Run;
	};
}