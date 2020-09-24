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

#include "NaiveBayes.src.h"

namespace FSMTP::Spam::NaiveBayes {
	Classifier::Classifier():
		m_MinimumLength(3), m_DictSize(2000), m_Logger("Classifier", LoggerLevel::DEBUG), m_Alpha(1)
	{}

	Classifier &Classifier::prepareHam(const string &raw) {
		return this->prepare(raw, this->m_HamWords, this->m_HamWordCount);
	}

	Classifier &Classifier::prepareSpam(const string &raw) {
		return this->prepare(raw, this->m_SpamWords, this->m_SpamWordCount);
	}

	Classifier &Classifier::prepareBoth(const string &spam, const string &ham) {
		future<Classifier &> f0 = async(launch::async, &Classifier::prepareHam, this, ham);
		future<Classifier &> f1 = async(launch::async, &Classifier::prepareSpam, this, spam);
		
		f0.get();
		f1.get();
		return *this;
	}

	Classifier &Classifier::prepare(const string &raw, vector<struct Word> &target, double &wordCount) {
		regex regexp(R"(\w+)");

		for (sregex_iterator it(raw.begin(), raw.end(), regexp), it_end; it != it_end; ++it) {
			// Transforms the selected word to lowercase, since we're not interested in the 
			//  capitalization of each word, after which we check the word's length, and ignore
			//  it if it is lower then the minimum length
			string lower = (*it)[0];
			transform(lower.begin(), lower.end(), lower.begin(), [](const char c) {
				return tolower(c);
			});
			if (lower.length() < m_MinimumLength) continue;
			else wordCount += 1.0f;

			// Attempts to find the existing word in our vector
			vector<struct Word>::iterator existing = find_if(target.begin(), target.end(), [&](const struct Word &w) {
				return (lower == w.word);
			});

			// Checks if there is already an existing entry of the word, if not
			//  create one, else increment the number of occurences of the word
			if (existing == target.end()) 
				target.push_back(Word {
					1, lower
				});
			else ++existing->occurrences;
		}


		return *this;
	}

	Classifier &Classifier::compute() {
		DEBUG_ONLY(Timer("Compute", this->m_Logger));

		// Gets the most important words from the vector
		auto s = [&](vector<struct Word> &target) {
			sort(target.begin(), target.end(), [](const struct Word &l, const struct Word &r) {
				return (l.occurrences > r.occurrences);
			});
		};
		s(this->m_SpamWords);
		s(this->m_HamWords);

		// If the size of the words vector exceeds the max size of the dictionary
		//  we will erase the less relevant words of it, the ones which do not matter
		//  much since they're not used that oftem
		if (this->m_SpamWords.size() > this->m_DictSize)
			this->m_SpamWords.erase(this->m_SpamWords.begin() + this->m_DictSize, this->m_SpamWords.end());
		if (this->m_HamWords.size() > this->m_DictSize)
			this->m_HamWords.erase(this->m_HamWords.begin() + this->m_DictSize, this->m_HamWords.end());

		return *this;
	}

	enum Prediction Classifier::predict(const string &raw) {
		Logger &logger = this->m_Logger;
		regex regexp(R"(\w+)");
		
		// Calculates the total number of messages, after which we calculate
		//  the initial values for both spam and ham
		double count = static_cast<double>(this->m_HamCount + this->m_SpamCount) + this->m_Alpha;
		double hig = static_cast<double>(this->m_HamCount) / count;
		double sig = static_cast<double>(this->m_SpamCount) / count;

		DEBUG_ONLY(logger << "Prior probability { Ham: " << hig << ", Spam: " << sig << " }" << ENDL);

		// Starts splitting the message into words
		for (sregex_iterator it(raw.begin(), raw.end(), regexp), it_end; it != it_end; ++it) {
			string lower = (*it)[0];
			transform(lower.begin(), lower.end(), lower.begin(), [](const char c) { return tolower(c); });

			// Updates the ham value with the existing words
			vector<struct Word>::iterator existing = find_if(this->m_HamWords.begin(), this->m_HamWords.end(), [&](const struct Word &w) {
				return (lower == w.word);
			});
			if (existing != this->m_HamWords.end()) {
				hig *= (static_cast<double>(existing->occurrences + this->m_Alpha) / this->m_HamWordCount);
			}

			// Updates the ham spam with the existing words
			existing = find_if(this->m_SpamWords.begin(), this->m_SpamWords.end(), [&](const struct Word &w) {
				return (lower == w.word);
			});
			if (existing != this->m_SpamWords.end()) {
				sig *= (static_cast<double>(existing->occurrences + this->m_Alpha) / this->m_SpamWordCount);
			}
		}

		// Checks the predictions result, and returns them, while printing the
		//  info to the console
		DEBUG_ONLY(logger << "Probability: { Ham: " << hig << ", Spam: " << sig << " }" << ENDL);
		if (hig > sig) {
			DEBUG_ONLY(logger << "Classifier result: Ham" << ENDL);
			return Prediction::Ham;
		} else {
			DEBUG_ONLY(logger << "Classifier result: Spam" << ENDL);
			return Prediction::Spam;
		}
	}

	Classifier &Classifier::print() {
		auto printWords = [&](const char *name, const vector<struct Word> &words) {
			cout << name << ": " << endl;
			size_t i = 0;
			for_each(words.begin(), words.end(), [&](const struct Word &w) {
				cout << '\t' << i++ << " -> { word: '" << w.word << "', occurrences: " << w.occurrences << " }" << endl;
			});
		};

		// Prints the words of both the categories
		printWords("Ham", this->m_HamWords);
		printWords("Spam", this->m_SpamWords);

		return *this;
	}

	Classifier &Classifier::setCount(size_t spamCount, size_t hamCount) {
		this->m_SpamCount = spamCount;
		this->m_HamCount = hamCount;
		return *this;
	}

	Classifier::~Classifier() {}
}