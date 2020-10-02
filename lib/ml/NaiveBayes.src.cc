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

namespace FSMTP::ML {
    NaiveBayes::NaiveBayes(double a):
        m_Alpha(a)
    {}

	NaiveBayes &NaiveBayes::fit(const vector<pair<size_t, vector<string>>> &categories) {
		
	}

	NaiveBayes::~NaiveBayes() = default;	
}
