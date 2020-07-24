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

#include "DNSServer.src.h"

namespace FSMTP::DNS
{
	DNSServer::DNSServer(const int32_t port):
		s_Socket(port), s_Run(true), s_Running(false)
	{
		// Starts listening
		this->s_Socket.startAcceptorSync(&this->s_Run, &this->s_Running, 
			DNSServer::acceptorCallback, this);
	}

 void DNSServer::acceptorCallback(DNSServerSocket *server, void *u)
 {
 		DNSHeader header;
 		struct sockaddr_in clientAddr;
 		int32_t clientAddrLen = sizeof(struct sockaddr_in);
 		int32_t rc;

 		// Reads data from the client, and gets the client address
 		rc = recvfrom(server->s_SocketFD, reinterpret_cast<char *>(header.d_Buffer), 
 			sizeof(header.d_Buffer), MSG_WAITALL, 
 			reinterpret_cast<struct sockaddr *>(&clientAddr),
 			reinterpret_cast<socklen_t *>(&clientAddrLen)
 		);

 		if (rc >= 12)
 		{
 			// Creates the logger, and prints that the client is connected
		 	char prefix[128];
		 	sprintf(prefix, "%s:%s", "DNSCB", inet_ntoa(clientAddr.sin_addr));
		 	Logger logger(prefix, LoggerLevel::INFO);
		 	logger << "Client connected !" << ENDL;

		 	// Logs the request
		 	header.log(logger);
 		}
 	}
}
