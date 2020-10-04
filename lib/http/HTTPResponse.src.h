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

#ifndef _LIB_HTTP_RESPONSE_H
#define _LIB_HTTP_RESPONSE_H

#include "../default.h"
#include "../mime/types.src.h"
#include "HTTPRequest.src.h"

namespace FSMTP::HTTP {
  class HTTPResponse {
  public:
		HTTPResponse(HTTPRequest &req);

		HTTPResponse &sendHead();
		HTTPResponse &sendHeaders();

		HTTPResponse &sendFile(const string &file);
		HTTPResponse &sendText(const string &text, MIME::FileTypes type);

		~HTTPResponse();
  private:
		HTTPRequest &m_Request;	
		vector<MIME::MIMEHeader> m_Headers;
  };
}

#endif
