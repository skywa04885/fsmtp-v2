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

#ifndef _LIB_HTTP_REQUEST_H
#define _LIB_HTTP_REQUEST_H

#include "../default.h"
#include "../general/Logger.src.h"
#include "../mime/mimev2.src.h"

#include "HTTP.src.h"

namespace FSMTP::HTTP {
    class HTTPRequest {
    public:
        HTTPRequest();

        HTTPRequest &parse(const string &raw);
        HTTPRequest &parseHeaders(strvec_it begin, strvec_it end);
        HTTPRequest &parseHead(const string &raw);

        HTTPRequest &print(Logger &logger);

        const HTTPUri &getURI();
        HTTPMethod getMethod();
        HTTPVersion getVersion();
        HTTPConnection getConnection();
        HTTPAllowedEncoding getAcceptEncoding();
        const vector<MIME::MIMEHeader> &getHeaders();

        string getUserAgent();

        ~HTTPRequest();
    private:
        HTTPUri m_URI;
        HTTPHead m_Head;
        HTTPConnection m_Connection;
        vector<MIME::MIMEHeader> m_Headers;
    };
}

#endif
