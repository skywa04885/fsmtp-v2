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

#ifndef _SRC_MIME_MIME2_H
#define _SRC_MIME_MIME2_H

#include "../default.h"

#include "../general/Logger.src.h"
#include "../general/Timer.src.h"
#include "../general/encoding.src.h"
#include "../models/Email.src.h"

using namespace FSMTP::Models;

namespace FSMTP::MIME {
  struct MIMEHeader {
    string key, value;
  };

  string decodeHeader(const string &raw);

  string decodeMIMEContent(
    strvec_it from, strvec_it to, const EmailTransferEncoding encoding
  );
  
  tuple<strvec_it, strvec_it, strvec_it, strvec_it> splitMIMEBodyAndHeaders(strvec_it from, strvec_it to);

  vector<MIMEHeader> parseHeaders(strvec_it from, strvec_it to);
  vector<MIMEHeader> _parseHeaders(strvec_it from, strvec_it to, bool lowerKey);  

  vector<string> splitHeaderBySemicolon(const string &raw);
  
  map<string, string> parseHeaderKeyValuePairs(strvec_it from, strvec_it to);

  tuple<EmailContentType, string, string> parseContentType(const string &raw);

  void parseMIMEDataFromHeaders(
    const string &key, const string &value, FullEmail &email
  );

  vector<tuple<strvec_it, strvec_it>> splitMultipartMessage(strvec_it from, strvec_it to, const string &boundary);

  void parseMIMERecursive(
    FullEmail &email, const size_t i, strvec_it from, strvec_it to
  );

 
  void parseMIME(const string &raw, FullEmail &email);

  vector<string> getMIMELines(const string &raw);

  string getStringFromLines(strvec_it from, strvec_it to);

  vector<string> joinHeaders(strvec_it from, strvec_it to);
};

#endif