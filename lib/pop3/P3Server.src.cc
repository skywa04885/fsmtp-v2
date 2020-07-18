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

#include "P3Server.src.h"

namespace FSMTP::POP3
{
	P3Server::P3Server(const bool secure):
		s_Socket((secure ? _POP3_PORT_SECURE : _POP3_PORT_PLAIN)),
		s_Logger("P3Server", LoggerLevel::INFO)
	{
		this->s_Socket.startListening(
			&this->s_Running,
			&this->s_Run,
			&P3Server::acceptorCallback
		);
	}

	void P3Server::acceptorCallback(std::unique_ptr<ClientSocket> client)
	{
		// Sends the initial greeting message
		client->sendResponse(true, POP3ResponseType::PRT_GREETING);

		for (;;)
		{
			P3Command command;

			try {
				std::string raw = client->readUntillCRLF();
				command.parse(raw);
			} catch (const SocketReadException &e)
			{
				break;
			}

			// Checks how to respond to the command
			switch (command.c_Type)
			{
				case POP3CommandType::PCT_QUIT:
				{
					goto pop3_session_end;
				}
			}
		}
	pop3_session_end:
		return;
	}

	/**
	 * Stops the pop3 server
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void P3Server::shutdown(void)
	{
		Logger &logger = this->s_Logger;

		logger << "POP3 Server wordt afgesloten ..." << ENDL;
		int64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
		this->s_Socket.shutdown(&this->s_Running, &this->s_Run);
		int64_t end = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
		logger << "POP3 Server afgesloten in " << end - start << " milliseconden" << ENDL;
	}
}
