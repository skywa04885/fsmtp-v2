
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

#include "P3ClientSocket.src.h"

extern Json::Value _config;

namespace FSMTP::POP3
{
	ClientSocket::ClientSocket(
		const int32_t s_SocketFD,
		const struct sockaddr_in s_SocketAddr
	):
		s_SocketFD(s_SocketFD), s_SocketAddr(s_SocketAddr),
		s_Logger(
			std::string("POP3Client:") + inet_ntoa(s_SocketAddr.sin_addr),
			LoggerLevel::INFO
		), s_UseSSL(false)
	{
		Logger &logger = this->s_Logger;

		DEBUG_ONLY(logger << DEBUG << "Connected" << ENDL << CLASSIC);
	}

	ClientSocket::~ClientSocket(void)
	{
		Logger &logger = this->s_Logger;

		shutdown(this->s_SocketFD, SHUT_RDWR);
		logger << WARN << "Closed transmission channel" << ENDL;

		if (this->s_UseSSL)
		{
			SSL_CTX_free(this->s_SSLCtx);
			SSL_free(this->s_SSL);
		}
	}

	std::string ClientSocket::readUntillCRLF(void)
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
					std::string error = "SSL_peek() failed: ";
					error += strerror(errno);
					throw SocketReadException(EXCEPT_DEBUG(error));
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
				std::string error = "SSL_read() failed: ";
				error += strerror(errno);
				throw SocketReadException(EXCEPT_DEBUG(error));
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

	void ClientSocket::sendString(const std::string &raw)
	{
		int32_t rc;
		DEBUG_ONLY(this->s_Logger << DEBUG << "S->" << raw.substr(0, raw.size() - 2) << ENDL);

		if (this->s_UseSSL)
		{
			rc = SSL_write(this->s_SSL, raw.c_str(), raw.size());
			if (rc < 0)
			{
				ERR_print_errors_fp(stderr);
				std::string error = "SSL_write() failed: ";
				error += strerror(errno);
				throw SocketReadException(EXCEPT_DEBUG(error));
			}
		} else {
			rc = send(this->s_SocketFD, raw.c_str(), raw.size(), 0);
			if (rc <= 0)
			{
				std::string error = "send() failed: ";
				error += strerror(errno);
				throw SocketWriteException(EXCEPT_DEBUG(error));
			}
		}
	}

	void ClientSocket::sendResponse(const bool p_Ok, const POP3ResponseType p_Type)
	{
		P3Response response(p_Ok, p_Type);
		this->sendString(response.build());
	}

	int ClientSocket::readSSLPassphrase(char *buffer, int size, int rwflag, void *u)
	{
		// Reads the file and stores it inside the buffer
		// - if something goes wrong we simply throw an error
		FILE *f = fopen(_config["ssl_pass"].asCString(), "r");
		if (!f)
		{
			std::string error = "fopen() failed: ";
			error += strerror(errno);
			throw std::runtime_error(error);
		}

		fgets(buffer, size, f);
		return strlen(buffer);
	}

	/**
	 * Upgrades the client to an SSL socket
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void ClientSocket::upgrade(void)
	{
		Logger &logger = this->s_Logger;
		int32_t rc;

		logger << "Verbinding wordt beveiligd" << ENDL;

		// Creates the SSL context and throws error
		// - if something goes wrong
		const SSL_METHOD *sslMethod = SSLv23_server_method();
		this->s_SSLCtx = SSL_CTX_new(sslMethod);
		if (!this->s_SSLCtx)
			throw SocketSSLError(EXCEPT_DEBUG("Could not create SSL_CTx"));

		// Configures the SSL context with keys etcetera
		SSL_CTX_set_ecdh_auto(this->s_SSLCtx, 1);
		SSL_CTX_set_default_passwd_cb(this->s_SSLCtx, &ClientSocket::readSSLPassphrase);

		rc = SSL_CTX_use_certificate_file(this->s_SSLCtx, _config["ssl_cert"].asCString(), SSL_FILETYPE_PEM);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw SocketSSLError("Could not read cert");
		}

		rc = SSL_CTX_use_PrivateKey_file(this->s_SSLCtx, _config["ssl_key"].asCString(), SSL_FILETYPE_PEM);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw SocketSSLError("Could not read private key");
		}

		// Creates the ssl struct and binds it with the socket
		// - then we accept the secure connection
		this->s_SSL = SSL_new(this->s_SSLCtx);
		SSL_set_fd(this->s_SSL, this->s_SocketFD);

		rc = SSL_accept(this->s_SSL);
		if (rc <= 0)
		{
			ERR_print_errors_fp(stderr);
			throw SocketSSLError("Could not accept secure connection");
		}

		// Sets useSSL to true
		this->s_UseSSL = true;
		logger << "Verbinding beveiligd" << ENDL;
	}

	void ClientSocket::sendResponse(
		const bool p_Ok,
		const POP3ResponseType p_Type,
		const std::string &p_Message,
		std::vector<POP3Capability> *p_Capabilities,
		std::vector<POP3ListElement> *p_ListElements,
		void *p_U
	)
	{
		P3Response response(
			p_Ok,
			p_Type,
			p_Message,
			p_Capabilities,
			p_ListElements,
			p_U
		);
		this->sendString(response.build());
	}
}
