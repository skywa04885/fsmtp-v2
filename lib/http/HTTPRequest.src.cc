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
    HTTPRequest::HTTPRequest():
        m_Connection(HTTPConnection::ConnectionClose)
    {};

    HTTPRequest &HTTPRequest::parse(const string &raw) {
        vector<string> lines = MIME::getMIMELines(raw);
        if (lines.size() <= 0)
            throw runtime_error(EXCEPT_DEBUG("HTTPRequest is empty"));

        // Parses the head of the http request
        this->parseHead(lines[0]);
        lines.erase(lines.begin(), lines.begin() + 1);

        // Checks the HTTP version, and if we even should parse
        //  the headers from it
        if (this->m_Head.version == HTTPVersion::Http09) return *this;

        // Checks if there are any headers, if so start parsing
        //  them and store them into the current request
        if (lines.size() <= 0) return *this;
        this->parseHeaders(lines.begin(), lines.end());

        // Loops over the headers, and checks if there is valueable
        //  information in them, about the request
        for_each(this->m_Headers.begin(), this->m_Headers.end(), [&](const MIME::MIMEHeader &h) {
            if (h.key == "host")
                this->m_URI.hostname = h.value;
            else if (h.key == "connection")
                this->m_Connection = __httpConnectionFromString(h.value);
        });

        return *this;
    }

    HTTPRequest &HTTPRequest::parseHeaders(strvec_it begin, strvec_it end) {
        this->m_Headers = MIME::_parseHeaders(begin, end, true);
        return *this;
    }

    HTTPRequest &HTTPRequest::parseHead(const string &raw) {
        this->m_Head = HTTPHead::parse(raw);
        this->m_URI = HTTPUri::parse(this->m_Head.uri);
        return *this;
    }

    HTTPRequest &HTTPRequest::print(Logger &logger) {
        logger << DEBUG;

        logger << "HTTPRequest {" << ENDL;
        logger << "\tHead: " << ENDL;
        logger << "\t\tMethod: " << __httpMethodToString(this->m_Head.method) << ENDL;
        logger << "\t\tVersion: " << __httpVersionToString(this->m_Head.version) << ENDL;
        logger << "\t\tURI: " << this->m_Head.uri << ENDL;

        logger << "\tURI: " << ENDL;
        logger << "\t\tPath: " << this->m_URI.path << ENDL;
        logger << "\t\tHostname: " << this->m_URI.hostname << ENDL;
        logger << "\t\tSearch: " << this->m_URI.search << ENDL;
        logger << "\t\tProtocol: " << __httpProtocolToString(this->m_URI.protocol) << ENDL;
        logger << "\t\tConnection: " << __httpConnectionToString(this->m_Connection) << ENDL;
        
        logger << ENDL << "\tHeaders: " << ENDL;

        size_t i = 0;
        for_each(this->m_Headers.begin(), this->m_Headers.end(), [&](const MIME::MIMEHeader &h) {
            logger << "\t\t" << i++ << " -> MIMEHeader { key: '" << h.key 
                << ", value: '" << h.value << "' }" << ENDL;
        });

        logger << '}' << ENDL;
        logger << CLASSIC;

        return *this;
    }

    const HTTPUri &HTTPRequest::getURI()
    { return this->m_URI; }
    
    HTTPMethod HTTPRequest::getMethod()
    { return this->m_Head.method; }
    
    HTTPVersion HTTPRequest::getVersion()
    { return this->m_Head.version; }

    HTTPConnection HTTPRequest::getConnection()
    { return this->m_Connection; }

    const vector<MIME::MIMEHeader> &HTTPRequest::getHeaders()
    { return this->m_Headers; }

    HTTPAllowedEncoding HTTPRequest::getAcceptEncoding() {
        HTTPAllowedEncoding result;
        
        auto it = find_if(this->m_Headers.begin(), this->m_Headers.end(), [&](const MIME::MIMEHeader &h) {
            return (h.key == "accept-encoding");
        });

        // Checks if the header was found, if so start parsing the header
        //  and making sense of the tokens
        if (it != this->m_Headers.end()) {
            auto &raw = it->value;
            size_t start = 0, end = raw.find_first_of(',');
            for (;;) {
                // Gets the substring, checks if it is empty after which
                //  we check which type of compression is allowed
                string seg = raw.substr(start, end - start);
                if (!seg.empty()) {
                    if (*seg.begin() == ' ') seg.erase(seg.begin(), seg.begin() + 1);
                    if (*(seg.end() - 1) == ' ') seg.pop_back();

                    transform(seg.begin(), seg.end(), seg.begin(), [](const char c) {
                        return tolower(c);
                    });

                    if (seg == "compress") result.setCompress();
                    else if (seg == "gzip") result.setGZIP();
                    else if (seg == "deflate") result.setDeflate();
                }

                // Checks if we're at the end, else we will get
                //  another token
                if (end == string::npos) break;
                start = end + 1;
                end = raw.find_first_of(',', start);
            }
        }

        return result;
    }

    string HTTPRequest::getUserAgent() {
        for (const MIME::MIMEHeader &h : this->m_Headers)
            if (h.key == "user-agent") return h.value;
    }

    HTTPRequest::~HTTPRequest() = default;
}
