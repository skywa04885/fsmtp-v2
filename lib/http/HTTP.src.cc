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

#include "HTTP.src.h"

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
      default: return nullptr;
    }
  }

  HTTPMethod __httpMethodFromString(string str) {
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
      default: return nullptr;
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
      default: return nullptr;
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

  const char *__httpConnectionToString(HTTPConnection con) {
    assert(con > HTTPConnection::ConnectionStart && con < HTTPConnection::ConnectionEnd);
    switch (con) {
      case HTTPConnection::ConnectionKeepAlive: return "keep-alive";
      case HTTPConnection::ConnectionClose: return "close";
      default: return nullptr;
    }
  }

  HTTPConnection __httpConnectionFromString(string raw) {
    transform(raw.begin(), raw.end(), raw.begin(), [&](const char c) {
      return tolower(c);
    });

    if (raw == "keep-alive") return HTTPConnection::ConnectionKeepAlive;
    else if (raw == "close") return HTTPConnection::ConnectionClose;
    else throw runtime_error(EXCEPT_DEBUG(
      "Invalid HTTP connection: '" + raw + '\''));
  }

  HTTPUri HTTPUri::parse(const string &raw) {
    HTTPUri result;

    // Checks if there are any search parameters
    //  if not just store the whole URI as path
    size_t sep = raw.find_first_of('?');
    if (sep == string::npos) {
      result.path = raw;
    } else {
      // Since there is an search, we will separate it
      //  from the path itself
      result.path = raw.substr(0, sep);
      result.search = raw.substr(++sep);
    }

    return result;
  }

  HTTPHead HTTPHead::parse(const string &raw) {
    HTTPHead result;

    size_t start = 0, end = raw.find_first_of(' ');
    for (size_t i = 0; i < 3; ++i) {
        string seg = raw.substr(start, end - start);
        switch (i) {
            case 0: result.method = __httpMethodFromString(seg); break;
            case 1: result.uri = seg; break;
            case 2: result.version = __httpVersionFromString(seg); break;
        }

        // Checks if we're at the end, else we proceed
        //  to the next token
        if (end == string::npos) break;
        start = end + 1;
        end = raw.find_first_of(' ', start);
    }

    return result;
  }

  const char *__getMessageForStatusCode(uint32_t code) {
    switch (code) {
      // > 100 && < 200
      case 100: return "Continue";
      case 101: return "Switching protocols";
      case 102: return "Processing";
      case 103: return "Early Hints";
      // > 200 && < 300
      case 200: return "OK";
      case 201: return "Created";
      case 202: return "Accepted";
      case 203: return "Non-Authoritative Information";
      case 204: return "No Content";
      case 205: return "Reset Content";
      case 206: return "Partial Content";
      case 226: return "IM Used";
      // > 300 && < 400
      case 300: return "Multiple Choises";
      case 301: return "Moved Permanently";
      case 302: return "Found";
      case 303: return "See Other";
      case 304: return "Not Modified";
      case 305: return "Use Proxy";
      case 306: return "Switch Proxy";
      case 307: return "Temporary Redirect";
      case 308: return "Permanent Redirect";
      // > 400 && < 500
      case 400: return "Bad Request";
      case 401: return "Unauthorized";
      case 402: return "Payment Required";
      case 403: return "Forbidden";
      case 404: return "Not Found";
      case 405: return "Method Not Allowed";
      case 406: return "Not Acceptable";
      case 407: return "Proxy Authentication Required";
      case 408: return "Request Timeout";
      case 409: return "Conflict";
      case 410: return "Gone";
      case 411: return "Length Required";
      case 412: return "Precondition Failed";
      case 413: return "Payload Too Large";
      case 414: return "URI Too Long";
      case 415: return "Unsupported Media Type";
      case 416: return "Range Not Satisfiable";
      case 417: return "Expectation Failed";
      case 418: return "I'm a teapot";
      case 421: return "Misdirected Request";
      case 425: return "Too Early";
      case 426: return "Upgrade Required";
      case 428: return "Precondition Required";
      case 429: return "Too Many Requests";
      case 431: return "Request Header Fields Too Large";
      case 451: return "Unavailable For Legal Reasons";
      // > 500
      case 500: return "Internal Server Error";
      case 501: return "Not Implemented";
      case 502: return "Bad Gateway";
      case 503: return "Service Unavailable";
      case 504: return "Gateway Timeout";
      case 505: return "HTTP Version Not Supported";
      case 506: return "Variant Also Negotiates";
      case 510: return "Not Extended";
      case 511: return "Network Authentication Required";
      // ?
      default: return "Unknown";
    }
  }

  const char *__httpCharsetToString(HTTPCharset charset) {
    assert(charset > HTTPCharset::HTTPCharsetStart && charset << HTTPCharset::HTTPCharsetStart);
    switch (charset) {
      case HTTPCharset::UTF8: return "utf-8";
      default: return nullptr;
    }
  }
}