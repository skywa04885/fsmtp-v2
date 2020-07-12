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

#include "encoding.src.h"

namespace FSMTP::Encoding
{
	/**
	 * Decodes 7 bit message
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	// TODO: Improve Quoted-Printable decoder
	std::string decodeQuotedPrintable(const std::string &raw)
	{
		std::string res;

		bool hexStarted = false;
		std::string hex;
		for (const char c : raw)
		{
			// Checks if an command started,
			// - if so set the command started boolean to
			// - true so we can start parsing
			if (c == '=')
			{
				hexStarted = true;
				continue;
			}

			if (hexStarted)
			{
				hex += c;

				if (hex.size() == 2)
				{
					// Appends the final char, and then performs
					// - the decode process, which appends it
					// - back to the result, after this we clear
					// - the result and set hex started to false
					std::cout << hex << std::endl;
					Encoding::HEX::decode(hex, res);
					
					hex.clear();
					hexStarted = false;
				}
			} else res += c;
		}

		return res;
	}

	/**
	 * Encodes message to quoted printable
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string encodeQuotedPrintable(const std::string &raw)
	{
		std::string res;

		// Loops over the chars and checks if they need to be
		// - encoded
		std::size_t i = 0;
		std::size_t lineLength = 0;
		for (const char c : raw)
		{
			if ((++lineLength) + 1 >= 76)
			{
				res += "=\n";
				lineLength = 0;
			}

			// Checks if we must encode the line
			if (c >= 33 && c <= 126 && c != 61 || c == '\n')
			{
				if (c == '\n') lineLength = 0;
				res += c;
			} else if (c >= 9 && c <= 32 && raw[i+1] != '\n')
			{
				res += c;
			}
			else
			{
				res += '=';
				HEX::encode(c, res);
				lineLength += 3;
			}

			// Increments the index
			i++;
		}

		return res;
	}
}