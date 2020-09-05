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
	 * Encodes an vector range to quoted printable
	 */
	string decodeQuotedPrintableRange(strvec_it from, strvec_it to) {
		string result;

		// Gets the binary value of an hex char
		//  will be called twice per sequence of decoding
		auto getBinaryHexValue = [](char c) {
			if (c >= 48 && c <= 57) return c - 48;		// If number, return the decimal value of the char
			if (c >= 65 && c <= 90) c = c + 32;			// If uppercase, make lowercase
			switch (c) {
				case 'a': return 10;
				case 'b': return 11;
				case 'c': return 12;
				case 'd': return 13;
				case 'e': return 14;
				case 'f': return 15;
			}
		};

		// Decodes an line of quoted-printable data
		//  each encoded char is in this way: =3D ( where 3D is a hex sequence )
		auto a = [&](const string &line) {
			string result;
			result.reserve(line.length());

			// Start looping over the string, while looping
			//  check for '=' if one is found, decode it and continue
			//  we use an buffer to optimize the concat operation, since
			//  it requires quite an amount of computational power
			char buffer[24] = {'\0'};
			size_t bufferSize = 0;
			for (string::const_iterator it = line.begin(); it != line.end(); ++it) {
				if (*it == '=') {
					// Skips the equals sign, and gets the two hex chars
					//  we place them into a null terminated buffer
					++it;
					char hexBuffer[3] = {*(it++), *it, '\0'};

					// Decodes the hex chars, and appends the new char to
					//  the buffer
					char decoded = getBinaryHexValue(hexBuffer[0]);
					decoded <<= 4;
					decoded |= getBinaryHexValue(hexBuffer[1]);

					// Appends the decoded char to the buffer
					buffer[bufferSize] = decoded;
					++bufferSize;
				} else {
					buffer[bufferSize] = *it;
					++bufferSize;
				}

				// Appends and flushes the buffer if the buffersize
				//  is too large
				if (bufferSize >= 24) {
					result += buffer;
					buffer[0] = '\0';
					bufferSize = 0;
				}
			}

			// Checks if there is anything left in the buffer
			//  if so put a null term at the current insertion index
			//  and concat it to the result
			if (bufferSize > 0) {
				buffer[bufferSize] = '\0';
				result += buffer;
			}
			
			cout << result << endl;

			return result;
		};

		// Removes the '=' from an line, if it is there
		//  this means in almost all cases that there is
		//  a line break
		auto b = [](const string &line) {
			if (line[line.length() - 1] == '=') return line.substr(0, line.length() - 1);
			else return line;
		};

		// Starts looping over the lines and joining them if
		//  required, each line will be decoded using a()
		for (strvec_it it = from; it != to; ++it) {
			const string &line = *it;

			if (*(line.end() - 1) == '=') result += a(b(line));
			else result += a(line) + "\r\n";
		}

		return result;		
	}

	// TODO: Improve Quoted-Printable decoder
	string decodeQuotedPrintable(const string &raw) {
		string res;

		bool hexStarted = false;
		string hex;
		for (const char c : raw) {
			// Checks if an command started,
			// - if so set the command started boolean to
			// - true so we can start parsing
			if (c == '=') {
				hexStarted = true;
				continue;
			}

			if (hexStarted) {
				hex += c;

				if (hex.size() == 2)
				{
					// Appends the final char, and then performs
					// - the decode process, which appends it
					// - back to the result, after this we clear
					// - the result and set hex started to false
					cout << hex << endl;
					Encoding::HEX::decode(hex, res);
					
					hex.clear();
					hexStarted = false;
				}
			} else res += c;
		}

		return res;
	}

	string encodeQuotedPrintable(const string &raw)
	{
		string res;
		res.reserve(raw.length());

		auto encodeLine = [&](const string &line) {
			string temp;

			bool end = false;
			size_t index = 0;
			for (const char c : line) {
				if (++index == line.length()) {
					end = true;
				}

				if (
					(c > 33 && c < 126 && c != '=') ||
					((c == 9 || c == 32) && !end)
				) {
					temp += c;
				} else if (((c == 9 || c == 32) && end)) {
					// Since it is a space or tab at the end, we follow it by a
					//  soft line break
					temp += c;
					temp += '=';
				} else {
					temp += '=';
					HEX::encode(c, temp);
				}
			}

			return temp;
		};

		stringstream stream(raw);
		string line;
		while (getline(stream, line, '\n')) {
			if (!line.empty() && line[line.length() - 1] == '\r') {
				line.pop_back();
			}
			
			if (!line.empty()) {
				string temp = encodeLine(line);

				size_t left = temp.length(), total = 0;
				while (left > 76) {
					string substr = temp.substr(total, 75);
					left -= substr.length();
					total += substr.length();
					res += substr + "=\r\n";
				}

				if (left > 0) {
					res += temp.substr(total) + "\r\n";
				}
			}
		}

		return res;
	}

	string escapeHTML(const string &raw) {
		string res;
		res.reserve(raw.length());

		for (const char c : raw) {
			switch (c) {
				case '&':
					res += "&amp;";
					break;
				case '\"':
					res += "&quot;";
					break;
				case '\'':
					res += "&apos;";
					break;
				case '<':
					res += "&lt;";
					break;
				case '>':
					res += "&gt;";
					break;
				default: res += c;
			}
		}

		return res;
	}
}