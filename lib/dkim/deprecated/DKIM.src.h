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

/*
=>
	All options and decisions for the building of this file is available
	in [RFC6476](https://tools.ietf.org/html/rfc6376)
=>
*/

#pragma once

#include "../default.h"
#include "../general/cleanup.src.h"
#include "../parsers/mime.src.h"
#include "../general/encoding.src.h"
#include "../general/Timer.src.h"
#include "DKIMHashes.src.h"

using namespace FSMTP::Parsers;
using namespace FSMTP::Cleanup;

namespace FSMTP::DKIM
{
	// Order: body, headers
	typedef enum : uint8_t
	{
		DAP_RELAXED_RELAXED = 0,
		DAP_RELAXED_SIMPLE,
		DAP_SIMPLE_RELAXED,
		DAP_SIMPLE_SIMPLE
	} DKIMAlgorithmPair;

	typedef struct
	{
		const char *s_Version;
		const char *s_Algo;
		const char *s_CanonAlgo;
		const char *s_Domain;
		const char *s_KeySelector;
		std::string s_BodyHash;
		std::string s_Signature;
		std::vector<std::string> s_Headers;
	} DKIMHeaderSegments;

	typedef struct
	{
		const char *c_PrivateKeyPath;
		const char *c_Domain;
		const char *c_KeySelector;
		DKIMAlgorithmPair c_Algo = DKIMAlgorithmPair::DAP_RELAXED_RELAXED;
	} DKIMConfig;

	DKIMAlgorithmPair algorithmPairFromString(const string &compare);

	/**
	 * Signs an raw email using the DKIM algorithm
	 *
	 * @Param {const std::string&} email
	 * @Param {const DKIMConfig &} config
	 * @Return {std::string}
	 */
	std::string sign(const std::string &email, const DKIMConfig &config);

	/**
	 * Verifies an email with possible DKIM headers
	 *
	 * @Param {const std::string &email}
	 * @Return {void}
	 */
	bool verify(const std::string &email);

	/**
	 * Canonicalizes an message body with the relaxed algorithm
	 *
	 * rewritten at: Fri sep 4
	 */
	std::string _canonicalizeBodyRelaxed(const string &raw);

	/**
	 * Canonicalizes the headers using the relaxed algorithm
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string _canonicalizeHeadersRelaxed(const std::string &raw, bool signingCheck = true);

	/**
	 * Checks if we should use an specified header
	 *
	 * @Param {const std::string &} key
	 * @Return {bool}
	 */
	bool shouldUseHeader(const std::string &key);

	/**
	 * Buids the DKIM header based on the segments, auto format
	 * - tells the function if we need to break the lines and format
	 * - it transport compatable
	 *
	 * @Param {const DKIMHeaderSegments &} segments
	 * @Param {bool} autoFormat
	 * @Return {std::string}
	 */
	std::string buildDKIMHeader(const DKIMHeaderSegments &segments, bool autoFormat);
}
