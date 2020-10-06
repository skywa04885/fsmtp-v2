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

#ifndef _LIB_HTTP_SERVER_H
#define _LIB_HTTP_SERVER_H

#include "../default.h"
#include "../general/Logger.src.h"
#include "../general/Global.src.h"
#include "../networking/sockets/ServerSocket.src.h"
#include "../networking/sockets/ClientSocket.src.h"
#include "../networking/sockets/SSLContext.src.h"
#include "../mime/types.src.h"

#include "HTTPResponse.src.h"
#include "HTTPRequest.src.h"
#include "HTTPService.src.h"

namespace FSMTP::HTTP {
	class HTTPServer {
	public:
		HTTPServer();

		HTTPServer &listenServer();
		HTTPServer &createContext();
		HTTPServer &connectDatabases();
		HTTPServer &startHandler(bool newThread);
		HTTPServer &pushService(const string &domain, const HTTPService &service);

		static void defaultCallback(HTTPRequest request, HTTPResponse response, shared_ptr<Sockets::ClientSocket> client);

		~HTTPServer();
	private:
		unique_ptr<Sockets::ServerSocket> m_HTTPS, m_HTTP;
		unique_ptr<Sockets::SSLContext> m_SSLContext;
		unordered_map<string, HTTPService> m_Services;
		Logger m_Logger;
	};
}

#endif

