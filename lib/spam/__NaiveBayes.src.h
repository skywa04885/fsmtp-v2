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

#ifndef _LIB_SPAM_NAIVE_BAYES_H
#define _LIB_SPAM_NAIVE_BAYES_H

#include "../default.h"
#include "../general/Timer.src.h"
#include "../general/Logger.src.h"

namespace FSMTP::Spam::NaiveBayes {
	struct Word {
		size_t occurrences;
		string word;
	};

	enum Prediction {
		Spam, Ham, Rejected
	};

	class Classifier {
	public:
		Classifier();

		Classifier &prepareHam(const string &raw);
		Classifier &prepareSpam(const string &raw);
		Classifier &prepareBoth(const string &spam, const string &ham);
		Classifier &prepare(const string &raw, vector<struct Word> &target, double &wordCount);
		Classifier &compute();
		Classifier &print();
		Classifier &setCount(size_t spamCount, size_t hamCount);
		enum Prediction predict(const string &raw);

		~Classifier();
	protected:
	private:
		vector<struct Word> m_SpamWords;
		vector<struct Word> m_HamWords;
		size_t m_Alpha, m_MinimumLength, m_DictSize;
		size_t m_HamCount, m_SpamCount;
		double m_HamWordCount, m_SpamWordCount;
		Logger m_Logger;
	};
}

#endif