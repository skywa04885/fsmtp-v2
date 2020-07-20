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

#include "IMAPServer.src.h"

namespace FSMTP::IMAP
{
	IMAPServer::IMAPServer(const int32_t plainPort, const int32_t securePort):
		s_Socket(plainPort, securePort), s_SecureRunning(false),
		s_PlainRunning(false), s_Run(true)
	{
		// Starts listening
		this->s_Socket.startListening(
			&this->s_PlainRunning,
			&this->s_SecureRunning,
			&this->s_Run,
			&IMAPServer::acceptorCallback,
			this
		);
	}

	void IMAPServer::acceptorCallback(
		std::unique_ptr<IMAPClientSocket> client,
		void *u
	)
	{
		// Sends the initial response
		client->sendResponse(
			true, 0,
			IMAPResponseType::IRT_GREETING,
			IMAPResponsePrefixType::IPT_OK,
			nullptr
		);

		// Starts the communication loop
		for (;;)
		{
			try {
				std::string raw = client->readUntilCRLF();
			} catch (const SocketReadException &e)
			{

			}
		}
	}
}