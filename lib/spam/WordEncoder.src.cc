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

#include "WordEncoder.src.h"

namespace FSMTP::Spam::Preprocessing {
	WordEncoder::WordEncoder() {

	}

	void WordEncoder::splitString(const string &raw, vector<string> &words) {
		regex regexp(R"(\w+)");
		for (sregex_iterator it(raw.begin(), raw.end(), regexp), it_end; it != it_end; ++it) {
			string word = (*it)[0];
			transform(word.begin(), word.end(), word.begin(), [](const char c) { return tolower(c); });
			words.push_back(word);
		}
	}

	WordEncoder &WordEncoder::fit(const vector<string> &words, function<bool(const string &)> filter) {
		size_t i = 0;
		for_each(words.begin(), words.end(), [&](const string &word) {
			if (this->m_Words.find(word) != this->m_Words.end()) ++this->m_Words[word].second;
			else if (filter(word)) this->m_Words[word] = make_pair<size_t, size_t>(i++, 1);
		});

		return *this;
	}

	WordEncoder &WordEncoder::print(Logger &logger) {
		size_t i = 0;
		for_each(this->m_Words.begin(), this->m_Words.end(), [&](const pair<string, pair<size_t, size_t>> &p) {
			logger << i++ << " -> { count: " << p.second.second 
				<< ", id: " << p.second.first 
				<< ", word: '" << p.first << "' }" << ENDL;
		});

		return *this;
	}

	WordEncoder::~WordEncoder() {

	}
}
