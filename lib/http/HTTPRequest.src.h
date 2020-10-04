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
#include "../parsers/mimev2.src.h"

namespace FSMTP::HTTP {
    enum HTTPMethod {
        MethodStart,
        Get, Post, Put, Head,
        Delete, Patch, Options,
        MethodEnd
    };

    const char *__httpMethodToString(HTTPMethod method);
    HTTPMethod __htmlMethodFromString(string str);

    enum HTTPVersion {
        VersionStart,
        Http09, Http10, Http11, Http2, Http3,
        VersionEnd
    };

    const char *__httpVersionToString(HTTPVersion version);
    HTTPVersion __httpVersionFromString(string str);

    enum HTTPProtocol {
        ProtocolStart,
        Https, Http,
        ProtocolEnd
    };

    const char *__httpProtocolToString(HTTPProtocol protocol);
    HTTPProtocol __httpProtocolFromString(string str);

    struct HTTPUri {
        string m_Hostname, m_Path, m_Search;
        HTTPProtocol m_Protocol;

        static HTTPUri parse(const string &raw);
    };

    class HTTPRequest {
    public:
        HTTPRequest();

        string build();
        string buildHead();
        string buildHeaders();

        HTTPRequest &parse(const string &raw);
        HTTPRequest &parseHeaders(strvec_it begin, strvec_it end);
        HTTPRequest &parseHead(const string &raw);

        const HTTPUri &getURI();
        HTTPMethod getMethod();
        HTTPVersion getVersion();

        ~HTTPRequest();
    private:
        HTTPUri m_URI;
        HTTPMethod m_Method;
        HTTPVersion m_Version;
    };
}

#endif
