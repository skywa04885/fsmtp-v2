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
#include "P3ClientSocket.src.h"

namespace FSMTP::POP3
{
	class ServerSocket
	{
	public:
		ServerSocket(int32_t port);

		void startListening(
			std::atomic<bool> *running,
			std::atomic<bool> *run,
			const std::function<void(std::unique_ptr<ClientSocket>, void *)> callback,
			void *u
		);

		void acceptingThread(
			std::atomic<bool> *running,
			std::atomic<bool> *run,
			const std::function<void(std::unique_ptr<ClientSocket>, void *)> callback,
			void *u
		);

		/**
		 * Stops the server
		 *
		 * @Param {std::atomic<bool> *} running
		 * @Param {std::atomic<bool> *} run
		 * @Return {void}
		 */
		void shutdown(std::atomic<bool> *running, std::atomic<bool> *run);
	private:
		struct sockaddr_in s_SocketAddr;
		int32_t s_SocketFD;
		Logger s_Logger;
	};
}
