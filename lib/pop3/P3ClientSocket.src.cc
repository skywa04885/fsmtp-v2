
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
		)
	{
		Logger &logger = this->s_Logger;

		DEBUG_ONLY(logger << DEBUG << "Connected" << ENDL << CLASSIC);
	}

	ClientSocket::~ClientSocket(void)
	{
		Logger &logger = this->s_Logger;

		shutdown(this->s_SocketFD, SHUT_RDWR);
		logger << WARN << "Closed transmission channel" << ENDL;
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
			rc = recv(this->s_SocketFD, buffer, sizeof(buffer), MSG_PEEK);
			if (rc < 0)
			{
				std::string error = "recv() failed: ";
				error += strerror(errno);
				throw SocketReadException(EXCEPT_DEBUG(error));
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
		rc = recv(this->s_SocketFD, buffer, ++i, 0);
		if (rc < 0)
		{
			std::string error = "recv() failed: ";
			error += strerror(errno);
			throw SocketReadException(EXCEPT_DEBUG(error));
		}

		// Returns the result
		std::string result(buffer, rc - 2);
		memset(buffer, 0, sizeof(buffer));
		DEBUG_ONLY(this->s_Logger << DEBUG << "C->" << result << ENDL);
		return result;
	}

	void ClientSocket::sendString(const std::string &raw)
	{
		DEBUG_ONLY(this->s_Logger << DEBUG << "S->" << raw.substr(0, raw.size() - 2) << ENDL);

		int32_t rc = send(this->s_SocketFD, raw.c_str(), raw.size(), 0);
		if (rc < 0)
		{
			std::string error = "send() failed: ";
			error += strerror(errno);
			throw SocketWriteException(EXCEPT_DEBUG(error));
		}
	}

	void ClientSocket::sendResponse(const bool p_Ok, const POP3ResponseType p_Type)
	{
		P3Response response(p_Ok, p_Type);
		this->sendString(response.build());
	}
}
