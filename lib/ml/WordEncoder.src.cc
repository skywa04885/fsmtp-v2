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

namespace FSMTP::ML {
    WordEncoder::WordEncoder():
        m_Count(0)
    {};

    WordEncoder &WordEncoder::fit(const vector<string> &words) {
        for_each(words.begin(), words.end(), [&](const string &word) {
            if (this->m_Map.find(word) == this->m_Map.end())
                this->m_Map.insert(make_pair(word, ++this->m_Count));
        });

        return *this;
    }

    WordEncoder &WordEncoder::fitWithFilter(const vector<string> &words, const function<bool(const string &)> &callback) {
        for_each(words.begin(), words.end(), [&](const string &word) {
            if (this->m_Map.find(word) == this->m_Map.end() && callback(word))
                this->m_Map.insert(make_pair(word, ++this->m_Count));
        });

        return *this;
    }

    size_t WordEncoder::encode(const string &word) {
        auto it = this->m_Map.find(word);
        if (it == this->m_Map.end()) return 0;
        else return it->second;
    }

    vector<size_t> WordEncoder::encode(const vector<string> &words) {
        vector<size_t> result = {};

        for_each(words.begin(), words.end(), [&](const string &word) {
            result.push_back(this->encode(word));
        });

        return result;
    }

    WordEncoder::~WordEncoder() = default;
}