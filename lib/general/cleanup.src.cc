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

#include "cleanup.src.h"

namespace FSMTP::Cleanup
{
	/**
	 * Reduces all white space patters to an single one
	 *
	 * @Param {const std::string &} raw
	 * @Param {std::string &} ret
	 * @Return {void}
	 */
	void reduceWhitespace(const std::string &raw, std::string &ret)
	{
		bool lww = false;
		for (const char &c : raw)
		{
			if (c == ' ')
			{
				if (lww) continue;
				ret += c;
				lww = true;
			} else
			{
				ret += c;
				lww = false;
			}
		}

		removeFirstAndLastWhite(ret);
	}

	/**
	 * Removes the first and last whitespace char from str
	 *
	 * @Param {std::string &} str
	 */
	void removeFirstAndLastWhite(std::string &str)
	{
		if (str.size() < 2) return;
		if (str[0] == ' ') str.erase(0, 1);
		if (str[str.size() - 1] == ' ') str.erase(str.size() - 1);
	}

	/**
	 * Removes string quotes from string if there
	 *
	 * @Param {std::stirng &} str
	 * @Return {void}
	 */
	void removeStringQuotes(std::string &str)
	{
		if (str.size() < 2) return;
		if (str[0] == '"') str.erase(0, 1);
		if (str[str.size() - 1] == '"') str.erase(str.size() - 1);	
	}
}