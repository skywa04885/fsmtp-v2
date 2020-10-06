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
    m_Client(client), m_Sent(false), m_TransferEncoding(HTTPEncoding::Chunked),
    m_Connection(HTTPConnection::ConnectionClose)
  {
    this->m_Version = req.getVersion();

    // Gets the accept encoding from the request, after which
    //  we will check how to encode the message
    auto acceptEncoding = req.getAcceptEncoding();
    if (acceptEncoding.getGZIP()) this->m_ContentEncoding = HTTPEncoding::GZIP;
    else if (acceptEncoding.getDeflate()) this->m_ContentEncoding = HTTPEncoding::Deflate;
    else if (acceptEncoding.getCompress()) this->m_ContentEncoding = HTTPEncoding::Compress;
    else this->m_ContentEncoding = HTTPEncoding::Identity;
  }

  HTTPResponse &HTTPResponse::sendHead(int32_t code) {
    char response[98];
    sprintf(response, "%s %d %s\r\n", __httpVersionToString(this->m_Version), 
      code, __getMessageForStatusCode(code));
    this->m_Client->write(response);
    return *this;
  }

  HTTPResponse &HTTPResponse::sendHeaders() {
    this->m_Client->write(Builders::buildHeaders(this->m_Headers, 128));
    this->m_Client->write("\r\n");
    return *this;
  }

  HTTPResponse &HTTPResponse::sendChunk(const char *data, size_t len) {
    // Checks how we need to send the message, this may be chunked or identity
    //  if not throw an error
    assert(this->m_TransferEncoding == HTTPEncoding::Identity || this->m_TransferEncoding == HTTPEncoding::Chunked);
    switch (this->m_TransferEncoding) {
      case HTTPEncoding::Identity: this->m_Client->write(data, len); break;
      case HTTPEncoding::Chunked: {
        char chunkHead[16];
        sprintf(chunkHead, "%zX\r\n", len);
        
        this->m_Client->write(chunkHead, strlen(chunkHead));
        this->m_Client->write(data, len);
        this->m_Client->write("\r\n", 2);
      }
      default: break;
    }
    return *this;
  }

  HTTPResponse &HTTPResponse::buildHeaders() {
    char dateBuffer[64];
    time_t rawTime;
    struct tm *timeInfo = nullptr;
    
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(dateBuffer, sizeof (dateBuffer), "%a, %-d %b %Y %T (%Z)", timeInfo);

    this->m_Headers.push_back(MIME::MIMEHeader {"Transfer-Encoding", 
      __httpEncodingToString(this->m_TransferEncoding)});
    this->m_Headers.push_back(MIME::MIMEHeader {"Content-Encoding", 
      __httpEncodingToString(this->m_ContentEncoding)});

    this->m_Headers.push_back(MIME::MIMEHeader {"Server", "FSMTP/HTTP by Skywa04885"});
    this->m_Headers.push_back(MIME::MIMEHeader {"Connection", __httpConnectionToString(this->m_Connection)});
    this->m_Headers.push_back(MIME::MIMEHeader {"Date", dateBuffer});

    return *this;
  }

  HTTPResponse &HTTPResponse::sendFile(int32_t code, const string &file) {
  // Attempts to get the file extension, so we can check what
    //  content type we will send to the client, by default binary
    size_t sep = file.find_last_of('.');
    MIME::FileTypes type = MIME::FileTypes::Binary;
    if (sep != string::npos) type = MIME::__extensionToFileType(file.substr(sep));

    this->sendFile(code, file, type);
  }

  HTTPResponse &HTTPResponse::sendFile(int32_t code, const string &file, MIME::FileTypes type) {    
    assert(!this->m_Sent);
    this->m_Sent = true;

    // Creates the file stream, and gets the length so we can
    //  build the content type header
    ifstream fileStream(file, ios_base::in);
    DEFER(fileStream.close());
    if (!fileStream.is_open())
      throw runtime_error(EXCEPT_DEBUG(
        "Failed to open file: '" + file + '\''));

    // Gets the length of the file, by going to the end
    //  and then getting the seek position
    fileStream.seekg(0, ios::end);
    size_t fileSize = fileStream.tellg();
    fileStream.seekg(0, ios::beg);

    // Builds the headers, such as the content type
    //  and content length
    this->m_Headers.push_back(MIME::MIMEHeader { "Content-Type", MIME::__fileTypeToString(type) });
    this->m_Headers.push_back(MIME::MIMEHeader { "Content-Length", to_string(fileSize) });

    // Sends the headers, after which we will start
    //  to write the rest of the file
    this->buildHeaders()
      .sendHead(code)
      .sendHeaders();

    // Checks the content encoding, and compresses the data
    //  if required, else just send plain
    switch (this->m_ContentEncoding) {
      case HTTPEncoding::Identity: {
        while (fileStream.good()) {
          fileStream.read(this->m_WriteBuffer, sizeof (this->m_WriteBuffer));
          this->sendChunk(this->m_WriteBuffer, fileStream.gcount()); break;
        }

        break;
      }
      case HTTPEncoding::GZIP: {
        this->sendGZIP(fileStream);
        this->m_Client->write("0\r\n\r\n");
        break;
      }
      case HTTPEncoding::Deflate: {
        this->sendDeflate(fileStream);
        this->m_Client->write("0\r\n\r\n");
        break;
      }
      default: break;
    }


    return *this;
  }

  HTTPResponse &HTTPResponse::sendText(int32_t code, const string &text, MIME::FileTypes type, HTTPCharset charset) {
    assert(!this->m_Sent);
    this->m_Sent = true;

    // Builds the content-type header for the text
    //  this will also contain an charset, since at 
    this->m_Headers.push_back(MIME::MIMEHeader { "Content-Type", MIME::__fileTypeToString(type) });
    this->m_Headers.push_back(MIME::MIMEHeader { "Content-Length", to_string(text.length()) });

    // Builds the headers, sends the head and after that
    //  we send the body
    this->buildHeaders()
      .sendHead(code)
      .sendHeaders();

    // Checks the content encoding, and compresses the data
    //  if required, else just send plain
    switch (this->m_ContentEncoding) {
      case HTTPEncoding::Identity: this->sendChunk(text.c_str(), text.length()); break;
      case HTTPEncoding::GZIP: {
        stringstream origin(text);
        this->sendGZIP(origin);
        this->m_Client->write("0\r\n\r\n");
        break;
      }
      case HTTPEncoding::Deflate: {
        stringstream origin(text);
        this->sendDeflate(origin);
        this->m_Client->write("0\r\n\r\n");
        break;
      }
      default: break;
    }

    return *this;
  }

  HTTPResponse::~HTTPResponse() = default;
}
