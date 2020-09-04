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

namespace FSMTP::Parsers {
  tuple<strvec_it, strvec_it, strvec_it, strvec_it> splitMIMEBodyAndHeaders(strvec_it from, strvec_it to) {
    strvec_it headersEnd;
    strvec_it bodyBegin;

    bool headersEnded = false;
    for (strvec_it it = from; it != to; ++it) {
      // Checks if the headers did not end, in that case
      //  we check if the line is empty, and then set headers
      //  ended to true, and store the corrent position
      if (!headersEnded) {
        if ((*it).empty()) {
          headersEnded = true;
          headersEnd = it;
        }
        continue;
      }

      // Since we now found the headers, we're going to look
      //  for the body. The start of the body is hit when the
      //  line is not empty, if it is hit we store the iterator
      //  and break the loop
      if (!(*it).empty()) {
        bodyBegin = it;
        break;
      }
    };

    return tuple(from, headersEnd, bodyBegin, to);
  }

  vector<EmailHeader> parseHeaders(strvec_it from, strvec_it to) {
    #ifdef _SMTP_DEBUG
    Logger logger("MIMEV2", LoggerLevel::DEBUG);
    Timer timer("parseHeaders()", logger);
    #endif

    vector<EmailHeader> headers = {};
    vector<string> joinedHeaders = {};

    // ================================
    // Joins headers, if required
    // ================================

    for (strvec_it it = from; it != to; it++) {
      // Checks if the next line contains an indention, if not
      //  just add it like it is a regular header
      if (it != to && ((*(it + 1))[0] == ' ' || (*(it + 1))[0] == '\t')) {
        string lineBuffer = *it;

        // Gets all the lines that belong to the current header, and removes
        //  the left-over whitespace
        while (it + 1 != to && ((*(it + 1))[0] == ' ' || (*(it + 1))[0] == '\t')) {
          // Checks if the previous iterator element is an semicolon
          //  if so we know that this is a header k/v pair which needs a space
          //  so we append it
          if (*((*it).end() - 1) == ';') lineBuffer += ' ';

          // Goes to the next line in the iterator, and removes the extra
          //  whitespace from it, after which we append it to the buffer
          ++it;
          (*it).erase(0, (*it).find_first_not_of(' '));
          (*it).erase(0, (*it).find_first_not_of('\t'));
          lineBuffer += *it;
        }

        // Appends the buffer to the joined headers
        joinedHeaders.push_back(lineBuffer);
      } else joinedHeaders.push_back(*it);
    }

    // ================================
    // Turns the headers into key/value
    //  pairs
    // ================================

    auto a = [](const string &str) {
      string res;
      res.reserve(str.length());

      // Removes all the duplicate whitespace
      //  prevw stands for previous whitespace
      bool prevw = false;
      for_each(str.begin(), str.end(), [&](char c) {
        if (c == ' ') {
          if (prevw) return;
          prevw = true;
        } else prevw = false;
        res += c;
      });

      if (*res.end() == ' ') res.pop_back();
      if (*res.begin() == ' ') res.erase(res.begin());

      return res;
    };

    // Loops over the joined headers, and turns them into key/value
    //  pairs, if a key is missing, throw syntax error
    size_t sep;
    for_each(joinedHeaders.begin(), joinedHeaders.end(), [&](const string &header) {
      sep = header.find_first_of(':');
      if (sep == string::npos)
        throw runtime_error(EXCEPT_DEBUG("Could not find k/v pair for header"));
      headers.push_back(EmailHeader {
        a(header.substr(0, sep)),
        a(header.substr(++sep))
      });
    });

    return headers;
  }

  void parseMIMERecursive(
    FullEmail &email, const size_t i, strvec_it from, strvec_it to
  ) {
    Logger logger(string("MIMEV2_ROUND:" + to_string(i)), LoggerLevel::DEBUG);

    strvec_it headersBegin;
    strvec_it headersEnd;
    strvec_it bodyBegin;
    strvec_it bodyEnd;

    tie(headersBegin, headersEnd, bodyBegin, bodyEnd) = splitMIMEBodyAndHeaders(from, to);

    // ================================
    // Parses the headers, and gets
    //  the default fields from them
    // ================================

    vector<EmailHeader> headers = parseHeaders(headersBegin, headersEnd);
    #ifdef _SMTP_DEBUG
    logger << "Found headers: " << ENDL;
    
    size_t j = 0;
    for_each(headers.begin(), headers.end(), [&](const EmailHeader &h) {
      logger << j++ << " -> '" << h.e_Key << "': '" << h.e_Value << '\'' << ENDL;
    });
    #endif

    // ================================
    // Starts parsing the body / body
    //  sections, based on the content
    //  type
    // ================================
  }

  void parseMIME(const string &raw, FullEmail &email) {
    vector<string> lines = {};

    stringstream stream(raw);
    string line;
    while (getline(stream, line, '\n')) {
      if (line[line.length() - 1] == '\r') line.pop_back();
      lines.push_back(line);
    }

    parseMIMERecursive(email, 0, lines.begin(), lines.end());
  }
}
