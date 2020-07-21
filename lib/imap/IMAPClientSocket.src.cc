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

#include "IMAPClientSocket.src.h"

namespace FSMTP::IMAP
{
	/**
	 * Default constructor for the imap client
	 *
	 * @Param {const struct sockaddr_in &} s_Addr
	 * @Param {const int32_t} s_SocketFD
	 * @Param {SSL *} s_SSL
	 * @Param {SSL_CTX *} s_SSLCTX
	 * @Return {void}
	 */
	IMAPClientSocket::IMAPClientSocket(
		const struct sockaddr_in &s_Addr,
		const int32_t s_SocketFD,
		SSL *s_SSL,
		SSL_CTX *s_SSLCTX
	):
		s_Addr(s_Addr), s_SocketFD(s_SocketFD), s_SSL(s_SSL), s_SSLCTX(s_SSLCTX),
		s_Logger(
			std::string("IMAPClientSock:") + inet_ntoa(s_Addr.sin_addr),
			LoggerLevel::INFO
		)
	{
		if (s_SSL != nullptr) this->s_UseSSL = true;
		else this->s_UseSSL = false;
	}

	/**
	 * Default client socket destructor, clears the SSL
	 * - when destroyed, since the server holds the SSL_CTX
	 * - we will not destroy it
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	IMAPClientSocket::~IMAPClientSocket(void)
	{
		Logger &logger = this->s_Logger;
		
		shutdown(this->s_SocketFD, SHUT_RDWR);
		logger << WARN << "Closed transmission channel" << ENDL;
		
		if (this->s_UseSSL)
		{
			SSL_free(this->s_SSL);
		}
	}

	/**
	 * Sends an string to the client
	 * 
	 * @Param {const std::stirng &} raw
	 * @Return {void}
	 */
	void IMAPClientSocket::sendString(const std::string &raw)
	{
		int32_t rc;

		DEBUG_ONLY(this->s_Logger << DEBUG << "S->" << raw.substr(0, raw.size() - 2) << ENDL);

		// Checks if we need to send it with ssl or not
		if (this->s_UseSSL)
		{
			rc = SSL_write(this->s_SSL, raw.c_str(), raw.size());
			if (rc <= 0)
			{
				ERR_print_errors_fp(stderr);
				throw SocketWriteException(EXCEPT_DEBUG("SSL_write() failed"));
			}
		} else
		{
			rc = send(this->s_SocketFD, raw.c_str(), raw.size(), 0);
			if (rc <= 0)
			{
				std::string error = "send() failed: ";
				error += strerror(errno);
				throw SocketWriteException(EXCEPT_DEBUG(error)); 
			}
		}
	}

	/**
	 * Upgrades the socket to an TLS socket
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void IMAPClientSocket::upgrade(void)
	{
		int32_t rc;
		DEBUG_ONLY(this->s_Logger << DEBUG << "Uitvoeren SSL upgrade ..." << ENDL << CLASSIC);

		// Creates the SSL struct, and then
		// - sets the file descriptor
		this->s_SSL = SSL_new(this->s_SSLCTX);
		SSL_set_fd(this->s_SSL, this->s_SocketFD);

		// Accepts the client
		rc = SSL_accept(this->s_SSL);
		if (rc <= 0)
		{
			SSL_free(this->s_SSL);
			ERR_print_errors_fp(stderr);
			throw SocketSSLError("SSL_accept() failed");
		}

		// Sets the useSSL to true
		this->s_UseSSL = true;
		DEBUG_ONLY(this->s_Logger << DEBUG << "Verbinding beveiligd !" << ENDL << CLASSIC);
	}

	/**
	 * Reads an string untill CRLF is reached
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string IMAPClientSocket::readUntilCRLF(void)
	{
		int32_t rc;
		char buffer[1024];
		std::string res;
		std::size_t i;

		// Gets the index of the newline
		for (;;)
		{
			if (this->s_UseSSL)
			{
				rc = SSL_peek(this->s_SSL, buffer, sizeof (buffer));
				if (rc <= 0)
				{
					// TODO: openssl error stuff
					ERR_print_errors_fp(stderr);
					throw SocketReadException(EXCEPT_DEBUG("SSL_peek() failed"));
				}
			} else
			{			
				rc = recv(this->s_SocketFD, buffer, sizeof(buffer), MSG_PEEK);
				if (rc <= 0)
				{
					std::string error = "recv() failed: ";
					error += strerror(errno);
					throw SocketReadException(EXCEPT_DEBUG(error));
				}
			}

			bool crlfFound = false;
			for (i = 0; i < rc; i++)
			{
				if (buffer[i] == '\n')
				{
					crlfFound = true;
					break;
				}
			}
			if (crlfFound) break;
		}

		// Reads the data untill the newline
		if (this->s_UseSSL)
		{
			rc = SSL_read(this->s_SSL, buffer, ++i);
			if (rc <= 0)
			{
				ERR_print_errors_fp(stderr);
				throw SocketReadException(EXCEPT_DEBUG("SSL_read() failed"));
			}
		} else
		{
			rc = recv(this->s_SocketFD, buffer, ++i, 0);
			if (rc <= 0)
			{
				std::string error = "recv() failed: ";
				error += strerror(errno);
				throw SocketReadException(EXCEPT_DEBUG(error));
			}
		}

		// Returns the result
		std::string result(buffer, rc - 2);
		memset(buffer, 0, sizeof(buffer));
		DEBUG_ONLY(this->s_Logger << DEBUG << "C->" << result << ENDL);
		return result;
	}
}