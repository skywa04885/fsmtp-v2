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

		// Reads the zone file
		this->s_Domains = readConfig();
	}

	const Domain &DNSServer::findDomain(const std::string &domain)
	{
		for (const Domain &dRef : this->s_Domains)
			if (dRef.d_Domain == domain) return dRef;
		throw std::runtime_error(EXCEPT_DEBUG("Could not find domain"));
	}

 	void DNSServer::acceptorCallback(DNSServerSocket *server, void *u)
 	{
 		DNSServer &dnsServer = *reinterpret_cast<DNSServer *>(u);
 		static int32_t clientAddrLen = sizeof(struct sockaddr_in);
 		DNSHeader request, response;
 		struct sockaddr_in clientAddr;
 		int32_t rc;

 		// Reads data from the client, and gets the client address,
 		// - then we set the length of the buffer
 		rc = recvfrom(server->s_SocketFD, reinterpret_cast<char *>(request.d_Buffer), 
 			sizeof(request.d_Buffer), MSG_WAITALL, 
 			reinterpret_cast<struct sockaddr *>(&clientAddr),
 			reinterpret_cast<socklen_t *>(&clientAddrLen)
 		);
 		request.d_BufferULen = rc;

 		if (rc >= 12)
 		{
 			// ================================================
 			// Prints some stuff
 			//
 			// Nothing much
			// ================================================

 			// Creates the logger, and prints that the client is connected
		 	char prefix[128];
		 	sprintf(prefix, "%s:%s", "DNSCB", inet_ntoa(clientAddr.sin_addr));
		 	Logger logger(prefix, LoggerLevel::INFO);
		 	logger << "Client connected !" << ENDL;

		 	// Logs the request
		 	request.log(logger);

			// ================================================
			// Builds response
			//
			// Processes the request, and builds the response
			// ================================================

			// Parses the questions, TODO: Support multiple query's
			bool secondQuery;
			std::size_t answerEndIndex;

			DNSQuestion question;
			std::tie(secondQuery, answerEndIndex) = question.parse(&request.d_Buffer[12]);
			
			question.log(logger);

			// Closes the request, and sets the values
			// - back to the normal / default ones
			response.clone(request, answerEndIndex + 12);
			response.setAA(true);
			response.setType(false);
			response.setArCount(0);

			// Reads the zone, so we can check which
			// - domains we have
			try {
				// Gets the domain, and sets the count
				const Domain &domain = dnsServer.findDomain(question.d_QName);
				response.setAsCount(domain.d_ARecords.size());

				// Builds the records and appends them
				for (const DNSRecord &r : domain.d_ARecords)
				{
					char ret[256];
					std::size_t len = r.build(ret);

					memcpy(&response.d_Buffer[response.d_BufferULen+1], ret, len);
					response.d_BufferULen += len;
				}
			} catch (const std::runtime_error &e)
			{ // TODO: Handle error

			}

			// Prints the response
			logger << " - RESPONSE - " << ENDL;
			response.log(logger);

	 		// Writes the response header
	 		sendto(server->s_SocketFD, response.d_Buffer, 
	 			response.d_BufferULen, MSG_CONFIRM, 
	 			reinterpret_cast<struct sockaddr *>(&clientAddr), clientAddrLen);

 	 		// Prints that the connection is closed
	 		logger << "Connection terminated, awaiting next" << ENDL;
 		}
 	}
}
