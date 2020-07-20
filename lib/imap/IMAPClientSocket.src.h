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
#include "IMAPResponse.src.h"

namespace FSMTP::IMAP
{
	class IMAPClientSocket
	{
	public:
		/**
		 * Default constructor for the imap client
		 *
		 * @Param {const struct sockaddr_in &} s_Addr
		 * @Param {const int32_t} s_SocketFD
		 * @Param {SSL *} s_SSL
		 * @Param {SSL_CTX *} s_SSLCTX
		 * @Return {void}
		 */
		IMAPClientSocket(
			const struct sockaddr_in &s_Addr,
			const int32_t s_SocketFD,
			SSL *s_SSL,
			SSL_CTX *s_SSLCTX
		);

		/**
		 * Default client socket destructor, clears the SSL
		 * - when destroyed, since the server holds the SSL_CTX
		 * - we will not destroy it
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		~IMAPClientSocket(void);

		/**
		 * Sends an string to the client
		 * 
		 * @Param {const std::stirng &} raw
		 * @Return {void}
		 */
		void sendString(const std::string &raw);

		/**
		 * Sends an response to the client
		 *
		 * @Param {const IMAPResponseType} r_Type
		 * @Param {const std::string} r_TagIndex
		 * @Param {const IMAPResponseStructure} r_Structure
		 * @Param {const IMAPResponsePrefixType} r_PrefType
		 * @Param {void *} r_U
		 * @Return {void}
		 */
		void sendResponse(
			const IMAPResponseStructure r_Structure,
			const std::string r_TagIndex,
			const IMAPResponseType r_Type,
			const IMAPResponsePrefixType r_PrefType,
			void *r_U
		);

		/**
		 * Sends an response to the client
		 *
		 * @Param {const IMAPResponseType} r_Type
		 * @Param {const std::string} r_TagIndex
		 * @Param {const IMAPResponseStructure} r_Structure
		 * @Param {const IMAPResponsePrefixType} r_PrefType
		 * @Param {const std::string &} r_Message
		 * @Param {void *} r_U
		 * @Return {void}
		 */
		void sendResponse(
			const IMAPResponseStructure r_Structure,
			const std::string r_TagIndex,
			const IMAPResponseType r_Type,
			const IMAPResponsePrefixType r_PrefType,
			const std::string &r_Message,
			void *r_U
		);

		/**
		 * Upgrades the socket to an TLS socket
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void upgrade(void);

		/**
		 * Reads an string untill CRLF is reached
		 *
		 * @Param {void}
		 * @Return {std::string}
		 */
		std::string readUntilCRLF(void);

		struct sockaddr_in s_Addr;
		bool s_UseSSL;
	private:
		Logger s_Logger;
		const int32_t s_SocketFD;
		SSL_CTX *s_SSLCTX;
		SSL *s_SSL;
	};
}