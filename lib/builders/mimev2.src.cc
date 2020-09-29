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

#include "mimev2.src.h"

namespace FSMTP::Builders {
    string foldHeader(const string &raw, size_t lineLength) {
        string result;

        // Gets the k/v pair
        size_t sep = raw.find_first_of(':');
        if (sep == string::npos)
            throw runtime_error(EXCEPT_DEBUG("Could not parse k/v pair from header: '" + raw + '\''));
        
        // Gets the key and the value based of the separator
        //  and cleans the header k/v after that
        string key = raw.substr(0, sep), val = raw.substr(++sep);
        if (*key.begin() == ' ') key.erase(key.begin(), key.begin() + 1);
        if (*(key.end() - 1) == ' ') key.pop_back();

        // Checks if the size of the key is larger than the maximum size
        //  if so we throw an error
        if (key.length() > lineLength)
            throw runtime_error(EXCEPT_DEBUG("Header key to large to fold: '" + key + '\''));

        // Puts the key in the result, and then makes the line size the 
        //  length of the key + ": "
        result += key + ": ";
        size_t currentLineLength = result.size();

        // Starts folding the header, by first separating it into small
        //  segments separated by ';' chars
        size_t start = 0, end = val.find_first_of(';');
        for (;;) {
            string seg = val.substr(start, end - start);

            // Checks if we should skip the current segment, else we will pop
            //  off some non-required whitespace
            if (seg.empty()) continue;
            if (*seg.begin() == ' ') seg.erase(seg.begin(), seg.begin() + 1);
            if (*(seg.end() - 1) == ' ') seg.pop_back();

            // Checks if the segment length + the current line length
            //  is bigger than the line length, if so we will process
            //  it first
            if (seg.length() > lineLength) {

            } else if (seg.length() + currentLineLength > lineLength) {

            } else {
                
            }

            // Checks if we're at the end of the string, and
            //  else continues to the next token
            if (end == string::npos) break;
            start = end + 1;
            end = val.find_first_of(';', start);
        }

        return result;
    }
}
