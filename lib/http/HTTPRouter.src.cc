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

#include "HTTPRouter.src.h"

namespace FSMTP::HTTP {
  HTTPRouter::HTTPRouter() = default;

	HTTPRouter &HTTPRouter::push(const string &path, const function<bool(const HTTPRequest&, 
    HTTPResponse&, shared_ptr<Sockets::ClientSocket>, const string&)> &callback)
	{ this->m_Callbacks.insert(make_pair(path, callback)); return *this; }

	bool HTTPRouter::handle(const string &path, const HTTPRequest &request,
		HTTPResponse &response, shared_ptr<Sockets::ClientSocket> client
	) {
		size_t pathSlash = path.find_first_of('/');
		for (auto &cb : this->m_Callbacks) {
			size_t slash = cb.first.find_first_of('/');

      // Checks if the slash position is equal, if not just continue since
      //  there will be no match, if both are npos just match, else match
      //  based on substring 
			if (pathSlash != slash) continue;
			else if (slash == string::npos || pathSlash == string::npos) {
				if (path == cb.first) return cb.second(request, response, client, path);
			} else if (path.substr(0, pathSlash) == cb.first.substr(0, slash))
        return cb.second(request, response, client, path.substr(pathSlash));
		}

    return false;
	}

	HTTPRouter::~HTTPRouter() = default;
}