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

#ifndef _LIB_DKIM_VERIFIER
#define _LIB_DKIM_VERIFIER

#include "../default.h"
#include "../general/cleanup.src.h"
#include "../parsers/mime.src.h"
#include "../models/Email.src.h"
#include "DKIM.src.h"
#include "../networking/DNS.src.h"

using namespace FSMTP::Parsers;
using namespace FSMTP::DNS;

namespace FSMTP::DKIM_Verifier {
	/**
	 * Parses an DKIM header, this is puts the k/v pairs into the specified map
	 */
	void parseDkimHeader(const string &raw, map<string, string> &header);

	/**
	 * Verifies an message which possibly contains some DKIM-Signature header
	 */
	bool verify(const string &raw);

	/**
	 * Gets the public key from an domain, which is needed to decode the signature
	 */
	void resolveRecord(const string &domain, const string &keySeletor);
}

#endif

