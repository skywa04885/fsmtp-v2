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

#ifndef _LIB_DKIM_CANON_H
#define _LIB_DKIM_CANON_H

#include "../default.h"
#include "../parsers/mimev2.src.h"

namespace FSMTP::DKIM {
  string relaxedBody(const string &raw);
  string simpleBody(const string &raw);

  string simpleHeaders(const string &raw, const vector<string> &filter);
  string relaxedHeaders(const string &raw, const vector<string> &filter);
};

#endif
