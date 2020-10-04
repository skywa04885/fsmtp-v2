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

#include "HTTPRequest.src.h"

namespace FSMTP::HTTP {
    const char *__httpMethodToString(HTTPMethod method) {
        assert(method > HTTPMethod::MethodStart && method < HTTPMethod::MethodEnd);
        switch (method) {
            case HTTPMethod::Get: return "GET";
            case HTTPMethod::Post: return "POST";
            case HTTPMethod::Put: return "PUT";
            case HTTPMethod::Head: return "HEAD";
            case HTTPMethod::Delete: return "DELETE";
            case HTTPMethod::Patch: return "PATCH";
            case HTTPMethod::Options: return "OPTIONS";
        }
    }

    HTTPMethod __htmlMethodFromString(string str) {
        transform(str.begin(), str.end(), str.begin(), [](const char c){
            return tolower(c);
        });

        if (str == "get") return HTTPMethod::Get;
        else if (str == "post") return HTTPMethod::Post;
        else if (str == "put") return HTTPMethod::Put;
        else if (str == "head") return HTTPMethod::Head;
        else if (str == "delete") return HTTPMethod::Delete;
        else if (str == "patch") return HTTPMethod::Patch;
        else if (str == "options") return HTTPMethod::Options;
        else throw runtime_error(EXCEPT_DEBUG(
            "HTTP Method not supported: '" + str + '\''));
    }

    const char *__httpVersionToString(HTTPVersion version) {
        assert(version > HTTPVersion::VersionStart && version < HTTPVersion::VersionEnd);
        switch (version) {
            case HTTPVersion::Http09: return "HTTP/0.9";
            case HTTPVersion::Http10: return "HTTP/1.0";
            case HTTPVersion::Http11: return "HTTP/1.1";
            case HTTPVersion::Http2: return "HTTP/2";
            case HTTPVersion::Http3: return "HTTP/3";
        }
    }

    HTTPVersion __httpVersionFromString(string str) {
        transform(str.begin(), str.end(), str.begin(), [](const char c){
            return tolower(c);
        });

        if (str == "http/0.9") return HTTPVersion::Http09;
        else if (str == "http/1.0") return HTTPVersion::Http10;
        else if (str == "http/1.1") return HTTPVersion::Http11;
        else if (str == "http/2") return HTTPVersion::Http2;
        else if (str == "http/3") return HTTPVersion::Http3;
        else throw runtime_error(EXCEPT_DEBUG(
            "HTTP version not supported: '" + str + '\''));
    }

    const char *__httpProtocolToString(HTTPProtocol protocol) {
        assert(protocol > HTTPProtocol::ProtocolStart && protocol < HTTPProtocol::ProtocolEnd);
        switch (protocol) {
            case HTTPProtocol::Http: return "http";
            case HTTPProtocol::Https: return "https";
        }
    }

    HTTPProtocol __httpProtocolFromString(string str) {
        transform(str.begin(), str.end(), str.begin(), [](const char c){
            return tolower(c);
        });

        if (str == "https") return HTTPProtocol::Https;
        else if (str == "http") return HTTPProtocol::Http;
        else throw runtime_error(EXCEPT_DEBUG(
            "HTTP protocol not supported: '" + str + '\''));
    }

    HTTPRequest::HTTPRequest() = default;

    string HTTPRequest::build() {

    }

    string HTTPRequest::buildHead() {

    }
    
    string HTTPRequest::buildHeaders() {

    }

    HTTPRequest &HTTPRequest::parse(const string &raw) {
        vector<string> lines = Parsers::getMIMELines(raw);
        if (lines.size() <= 0)
            throw runtime_error(EXCEPT_DEBUG("HTTPRequest is empty"));

        // Parses the head of the http request, after which
        //  we pop it of the final lines
        this->parseHead(lines[0]);
        lines.pop_back();

        // Checks the HTTP version, and if we even should parse
        //  the headers from it
        if (this->m_Version == HTTPVersion::Http09) return *this;

        // Since we checked if we should parse headers, we now check
        //  if there are any, after which we separate them from the body
        if (lines.size() <= 0) return *this;

        strvec_it headersBegin, headersEnd, bodyBegin, bodyEnd;
        tie(headersBegin, headersEnd, bodyBegin, 
            bodyEnd) = Parsers::splitMIMEBodyAndHeaders(lines.begin(), lines.end());

        // Parses the MIME headers into key / value pairs
    }

    HTTPRequest &HTTPRequest::parseHeaders(strvec_it begin, strvec_it end) {

    }

    HTTPRequest &HTTPRequest::parseHead(const string &raw) {
        size_t start = 0, end = raw.find_first_of(' ');

        for (size_t i = 0; i < 3; ++i) {
            string seg = raw.substr(start, end - start);
            switch (i) {
                case 0: this->m_Method = __htmlMethodFromString(seg); break;
                case 1: this->m_URI = HTTPUri::parse(seg); break;
            }

            // Checks if we're at the end, else we proceed
            //  to the next token
            if (end == string::npos) break;
            start = end + 1;
            end = raw.find_first_of(' ', start);
        }
    }

    const HTTPUri &HTTPRequest::getURI()
    { return this->m_URI; }
    
    HTTPMethod HTTPRequest::getMethod()
    { return this->m_Method; }
    
    HTTPVersion HTTPRequest::getVersion()
    { return this->m_Version; }

    HTTPRequest::~HTTPRequest() = default;
}
