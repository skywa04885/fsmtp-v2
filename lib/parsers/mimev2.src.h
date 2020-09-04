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

#ifndef _SRC_PARSERS_MIME2_H
#define _SRC_PARSERS_MIME2_H

#include "../default.h"

#include "../general/Logger.src.h"
#include "../general/Timer.src.h"
#include "../models/Email.src.h"

using namespace FSMTP::Models;

typedef vector<string>::iterator strvec_it;

namespace FSMTP::Parsers {
  tuple<strvec_it, strvec_it, strvec_it, strvec_it> splitMIMEBodyAndHeaders(strvec_it from, strvec_it to);

  vector<EmailHeader> parseHeaders(strvec_it from, strvec_it to);

  void parseMIMERecursive(
    FullEmail &email, const size_t i, strvec_it from, strvec_it to
  );
  void parseMIME(const string &raw, FullEmail &email);
};

#endif