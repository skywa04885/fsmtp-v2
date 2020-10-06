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

#ifndef _LIB_HTTP_SERVICE_H
#define _LIB_HTTP_SERVICE_H

#include "../default.h"
#include "HTTPRouter.src.h"

namespace FSMTP::HTTP {
	class HTTPService {
	public:
		HTTPService(bool forceHTTPS);

		HTTPService &pushRoute(const string &path, const function<bool(const HTTPRequest&, 
      HTTPResponse&, shared_ptr<Sockets::ClientSocket>, const string&)> &callback);

		bool handle(const string &path, const HTTPRequest &request,
      HTTPResponse &response, shared_ptr<Sockets::ClientSocket> client);

		~HTTPService();
	private:
		HTTPRouter m_Router;
		bool m_ForceHTTPS;
	};
}

#endif
