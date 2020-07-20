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
#include "IMAPClientSocket.src.h"

namespace FSMTP::IMAP
{
	class IMAPServerSocket
	{
	public:
		IMAPServerSocket(
			const int32_t plainPort,
			const int32_t securePort
		);

		void startListening(
			std::atomic<bool> *plainRunning,
			std::atomic<bool> *secureRunning,
			std::atomic<bool> *run,
			std::function<void(std::unique_ptr<IMAPClientSocket>, void *)> callback,
			void *u
		);

		void asyncAcceptorThreadSecure(
			std::atomic<bool> *secureRunning,
			std::atomic<bool> *run,
			std::function<void(std::unique_ptr<IMAPClientSocket>, void *)> callback,
			void *u	
		);

		void asyncAcceptorThreadPlain(
			std::atomic<bool> *plainRunning,
			std::atomic<bool> *run,
			std::function<void(std::unique_ptr<IMAPClientSocket>, void *)> callback,
			void *u	
		);

		/**
		 * Override constructor, destroys the ssl stuff
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		~IMAPServerSocket(void);
	private:
		SSL_CTX *s_SecureSSLCTX;
		const SSL_METHOD *s_SecureSSLMethod;
		struct sockaddr_in s_PlainSocketAddr;
		struct sockaddr_in s_SecureSocketAddr;
		int32_t s_SecureSocketFD;
		int32_t s_PlainSocketFD;
		Logger s_Logger;
	};
}
