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

#include "HTTPResponse.src.h"

namespace FSMTP::HTTP {
  HTTPResponse::HTTPResponse(HTTPRequest &req, shared_ptr<Sockets::ClientSocket> client):
    m_Request(req), m_Client(client), m_Sent(false),
    m_Connection(HTTPConnection::ConnectionClose)
  {}


  HTTPResponse &HTTPResponse::sendHead(int32_t code) {
    char response[98];
    sprintf(response, "%s %d %s", __httpVersionToString(this->m_Request.getVersion()), 
      code, __getMessageForStatusCode(code));
    this->m_Client->write(response);
    return *this;
  }

  HTTPResponse &HTTPResponse::sendHeaders() {
    this->m_Client->write(Builders::buildHeaders(this->m_Headers, 128));
    return *this;
  }

  HTTPResponse &HTTPResponse::sendBody(const char *data, size_t len) {
    this->m_Client->write("\r\n");
    this->m_Client->write(data, len);
    return *this;
  }

  HTTPResponse &HTTPResponse::buildHeaders() {
    char dateBuffer[64];
    time_t rawTime;
    struct tm *timeInfo = nullptr;
    
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(dateBuffer, sizeof (dateBuffer), "%a, %-d %b %Y %T (%Z)", timeInfo);

    this->m_Headers.push_back(MIME::MIMEHeader {"Transfer-Encoding", "identity"});
    this->m_Headers.push_back(MIME::MIMEHeader {"Content-Encoding", "identity"});

    this->m_Headers.push_back(MIME::MIMEHeader {"Server", "FSMTP/HTTP by Skywa04885"});
    this->m_Headers.push_back(MIME::MIMEHeader {"Connection", __httpConnectionToString(this->m_Connection)});
    this->m_Headers.push_back(MIME::MIMEHeader {"Date", dateBuffer});

    return *this;
  }

  HTTPResponse &HTTPResponse::sendText(int32_t code, const string &text, MIME::FileTypes type, HTTPCharset charset) {
    assert(!this->m_Sent);
    this->m_Sent = true;
    
    string contentType = MIME::__fileTypeToString(type);
    contentType += "; charset=";
    contentType += __httpCharsetToString(charset);
    this->m_Headers.push_back(MIME::MIMEHeader { "Content-Type", contentType });
    this->m_Headers.push_back(MIME::MIMEHeader { "Content-Length", to_string(text.length()) });

    this->buildHeaders()
      .sendHead(code)
      .sendHeaders()
      .sendBody(text.c_str(), text.length());

    return *this;
  }

  HTTPResponse::~HTTPResponse() = default;
}
