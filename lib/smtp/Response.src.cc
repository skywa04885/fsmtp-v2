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

#include "Response.src.h"

namespace FSMTP::SMTP
{
	/**
	 * Parses an server response into an string and code
	 *
	 * @Param {const std::string &} raw
	 * @Return {int32_t}
	 * @Return {std::string}
	 */
	std::tuple<int32_t, std::string> ServerResponse::parseResponse(const std::string &raw)
	{
		std::string clean;
		reduceWhitespace(raw, clean);

		std::size_t index = clean.find_first_of(' ');
		if (index == std::string::npos)
			return std::make_pair(std::stoi(clean), "");
		else return std::make_pair(
			std::stoi(clean.substr(0, index)),
			clean.substr(++index)
		);
	}
}