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

#include "hex.src.h"

namespace FSMTP::Encoding::HEX
{
	/**
	 * Turns an hex char to valid number
	 *
	 * @Param {const char} c
	 * @Return {char}
	 */
	char _reverseDict(const char c)
	{
	 switch (c)
    {
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      case 'A': return 10;
      case 'B': return 11;
      case 'C': return 12;
      case 'D': return 13;
      case 'E': return 14;
      case 'F': return 15;
      default: return 99;
    }
	}

	/**
	 * Turns an half char into hex char
	 *
	 * @Param {const char c}
	 * @Return {char}
	 */
	char _dict(const char c)
	{
		switch (c)
    {
      case 0: return '0';
      case 1: return '1';
      case 2: return '2';
      case 3: return '3';
      case 4: return '4';
      case 5: return '5';
      case 6: return '6';
      case 7: return '7';
      case 8: return '8';
      case 9: return '9';
      case 10: return 'A';
      case 11: return 'B';
      case 12: return 'C';
      case 13: return 'D';
      case 14: return 'E';
      case 15: return 'F';
      default: return '0';
    }
	}

	/**
	 * Decodes an string of hexadecimal characters
	 *
	 * @Param {const std::string &} raw
	 * @Param {std::string &} ret
	 * @Return {void}
	 */
	void decode(const std::string &raw, std::string &ret)
	{
		// Starts the decoding in pairs of one,
		// - this is because hex uses the first four
		// - bits of an octet and the last four
		char buffer[2];
		std::size_t bufferIndex = 0;
		for (const char c : raw)
		{
			buffer[bufferIndex++] = c;
			if (bufferIndex >= 2)
			{
				char c = _reverseDict(buffer[0]) & 0b00001111;
				c <<= 4;
				c |= _reverseDict(buffer[1]) & 0b00001111;

				ret += c;
				bufferIndex = 0;
			}
		}
	}

	/**
	 * Encodes an raw string into an hexadecimal string\
	 *
	 * @Param {const std::string &} raw
	 * @Param {std::string &} ret
	 * @Return {void}
	 */
	void encode(const std::string &raw, std::string &ret)
	{
		// Loops over the chars and encodes them
		for (const char c : raw)
		{
			ret += _dict(c & 0b00001111);
			ret += _dict((c & 0b11110000) >> 4);
		}
	}

	/**
	 * Encodes an octet into an hexadecimal string
	 *
	 * @Param {const char} raw
	 * @Param {std::string &} ret
	 * @Return {void}
	 */
	void encode(const char c, std::string &ret)
	{
		ret += _dict((c & 0b11110000) >> 4);
		ret += _dict(c & 0b00001111);
	}
}