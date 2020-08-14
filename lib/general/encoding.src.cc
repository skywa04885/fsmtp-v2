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