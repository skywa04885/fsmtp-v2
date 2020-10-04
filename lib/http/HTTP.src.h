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

#ifndef _LIB_HTTP_H
#define _LIB_HTTP_H

#include "../default.h"

namespace FSMTP::HTTP {
  enum HTTPMethod {
      MethodStart,
      Get, Post, Put, Head,
      Delete, Patch, Options,
      MethodEnd
  };

  const char *__httpMethodToString(HTTPMethod method);
  HTTPMethod __httpMethodFromString(string str);

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

  enum HTTPConnection {
      ConnectionStart,
      ConnectionKeepAlive, ConnectionClose,
      ConnectionEnd
  };

  const char *__httpConnectionToString(HTTPConnection con);
  HTTPConnection __httpConnectionFromString(string raw);

  struct HTTPUri {
      string hostname, path, search;
      HTTPProtocol protocol{HTTPProtocol::Http};

      static HTTPUri parse(const string &raw);
  };
}

#endif
