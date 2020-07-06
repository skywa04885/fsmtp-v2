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

#pragma once

#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <stdexcept>

#include "../general/macros.src.h"
#include "../general/logger.src.h"

namespace FSMTP::Parsers::MIME
{
	typedef struct
	{
		std::string h_Key;
		std::string h_Value;
	} MimeHeader;

	void parseHeaders(
		std::string raw,
		std::vector<MimeHeader> &headers, 
		const bool &removeMsGarbage
	);
}