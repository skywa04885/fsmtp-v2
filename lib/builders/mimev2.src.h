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

#ifndef _LIB_BUILDERS_MIMEV2_H
#define _LIB_BUILDERS_MIMEV2_H

#include "../default.h"
#include "../mime/mimev2.src.h"

namespace FSMTP::Builders {
	string buildHeaders(const vector<MIME::MIMEHeader> &headers, size_t maxLen);
	string foldHeader(const string &raw, size_t lineLength);
	string buildHeaderFromSegments(const char *label, const vector<pair<string, string>> &segments);
}

#endif
