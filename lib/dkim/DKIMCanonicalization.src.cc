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

#include "DKIMCanonicalization.src.h"

namespace FSMTP::DKIM {
  /**
   * : RFC 6376
   * 
   * The "relaxed" body canonicalization algorithm MUST apply the
   * following steps (a) and (b) in order:
   * 
   * a.  Reduce whitespace:
   *  1. Ignore all whitespace at the end of lines.  Implementations
   *     MUST NOT remove the CRLF at the end of the line.
   *  2. Reduce all sequences of WSP within a line to a single SP
   *     character.
   * 
   * b.  Ignore all empty lines at the end of the message body.  "Empty
   *     line" is defined in Section 3.4.3.  If the body is non-empty but
   *     does not end with a CRLF, a CRLF is added.  (For email, this is
   *     only possible when using extensions to SMTP or non-SMTP transport
   *     mechanisms.)
   */
  string relaxedBody(const string &raw) {
    vector<string> lines = MIME::getMIMELines(raw);
    
    // Reduces all the whitespace to a single whitespace char
    //  after which we remove the whitespace at the end of line
    for_each(lines.begin(), lines.end(), [&](string &line) {
      // Reduces all the whitespace to a single char
      string reducedLine;
      reducedLine.reserve(line.length());
      bool lww = false;
      for_each(line.begin(), line.end(), [&](char c) {
        if (c == '\t') c = ' ';

        if (c == ' ') {
          if (lww) return;
          else lww = true;
        } else lww = false;

        reducedLine += c;
      });

      // Removes the whitespace at the end of line, after which
      //  we update the current line
      if (*(reducedLine.end() - 1) == ' ') reducedLine.pop_back();
      line = reducedLine;
    });

    // Loops over the lines, and checks if it contains content, after which we remove
    //  the whitespace at the end of the document
    strvec_it lastValidLine = lines.begin();
    for (strvec_it it = lines.begin(); it != lines.end(); ++it) {
      if (!(*it).empty()) lastValidLine = it;
    }

    // Erases the lines at the end of the document, after which we
    //  will join the lines and return the result
    lines.erase(lastValidLine + 1, lines.end());
    return MIME::getStringFromLines(lines.begin(), lines.end());
  }

  /**
   * : RFC 6376
   * 
   * The "simple" body canonicalization algorithm ignores all empty lines
   * at the end of the message body.  An empty line is a line of zero
   * length after removal of the line terminator.  If there is no body or
   * no trailing CRLF on the message body, a CRLF is added.  It makes no
   * other changes to the message body.  In more formal terms, the
   * "simple" body canonicalization algorithm converts "*CRLF" at the end
   * of the body to a single "CRLF".
   * 
   * Note that a completely empty or missing body is canonicalized as a
   * single "CRLF"; that is, the canonicalized length will be 2 octets.
   */
  string simpleBody(const string &raw) {
    vector<string> lines = MIME::getMIMELines(raw);

    // Loops over the lines, and stores the index of the
    //  last line with content, so the rest may be removed
    strvec_it lastValidLine = lines.begin();
    for (strvec_it it = lines.begin(); it != lines.end(); ++it) {
      if (!(*it).empty()) lastValidLine = it;
    }
    
    // Erases the end of the lines vector, so removes the empty lines
    //  after which we join them and return them as an string
    lines.erase(lastValidLine + 1, lines.end());
    return MIME::getStringFromLines(lines.begin(), lines.end());
  }

  /**
   * : RFC 6376
   * 
   * The "simple" header canonicalization algorithm does not change header
   * fields in any way. Header fields MUST be presented to the signing or
   * verification algorithm exactly as they are in the message being
   * signed or verified. In particular, header field names MUST NOT be
   * case folded and whitespace MUST NOT be changed.
   */
  string simpleHeaders(const string &raw, const vector<string> &filter) {
    vector<string> lines = MIME::getMIMELines(raw);
    vector<string> result = {};

    for_each(lines.begin(), lines.end(), [&](const string &line) {
      if (line.empty()) return;

      // Gets the key from the header, to check if it is specified in the
      //  filter, and thus put in the final result
      size_t sep = line.find_first_of(':');
      if (sep == string::npos)
        throw runtime_error(EXCEPT_DEBUG("Could not parse k/v pair for header: '" + line + "'"));

      string key = line.substr(0, sep);
      transform(key.begin(), key.end(), key.begin(), [](const char c) { return tolower(c); });
      if (*key.begin() == ' ') key.erase(key.begin(), key.begin() + 1);
      if (*(key.end() - 1) == ' ') key.pop_back();

      // Checks if the key is present in the filter, if not
      //  skip the current key, else add them to the result
      if (find(filter.begin(), filter.end(), key) == filter.end()) return;
      result.push_back(line);
    });

    return MIME::getStringFromLines(result.begin(), result.end());
  }

  /**
   * : RFC 6376
   * 
   * The "relaxed" header canonicalization algorithm MUST apply the
   * following steps in order:
   * 
   *  1. Convert all header field names (not the header field values) to
   *     lowercase. For example, convert "SUBJect: AbC" to "subject: AbC".
   *  2. Unfold all header field continuation lines as described in
   *     RFC5322]; in particular, lines with terminators embedded in
   *     continued header field values (that is, CRLF sequences followed by
   *     WSP) MUST be interpreted without the CRLF.  Implementations MUST
   *     NOT remove the CRLF at the end of the header field value.
   *  3. Convert all sequences of one or more WSP characters to a single SP
   *     character.  WSP characters here include those before and after a
   *     line folding boundary.
   *  4. Delete all WSP characters at the end of each unfolded header field
   *     value.
   *  5. Delete any WSP characters remaining before and after the colon
   *     separating the header field name from the header field value. The
   *     colon separator MUST be retained.
   */
  string relaxedHeaders(const string &raw, const vector<string> &filter) {
    vector<string> lines = MIME::getMIMELines(raw);
    vector<pair<string, string>> result;
    vector<string> sortedResult = {};

    // Processes the lines
    for_each(lines.begin(), lines.end(), [&](const string &line) {
      // Gets the separator index, and if this fails
      //  we throw an runtime_error
      size_t sep = line.find_first_of(':');
      if (sep == string::npos)
        throw runtime_error(EXCEPT_DEBUG("Failed to pare k/v of header: '" + line + '\''));

      // Gets the key and value from the header, after which we remove the extra whitespace
      //  from the key, and check if it is in the filter
      string key = line.substr(0, sep), val = line.substr(++sep);
      transform(key.begin(), key.end(), key.begin(), [](const char c) { return tolower(c); });
      if (*key.begin() == ' ') key.erase(key.begin(), key.begin() + 1);
      if (*(key.end() - 1) == ' ') key.pop_back();

      // Checks if the key is in the filter, else we just return
      //  and do not process the header any further
      if (find(filter.begin(), filter.end(), key) == filter.end()) return;

      // Reduces all the whitespace in the value to a single whitespace occurence
      //  after which we will remove the suffix whitespace
      string reducedValue;
      reducedValue.reserve(line.length());
      bool lww = false;
      for_each(val.begin(), val.end(), [&](const char c) {
        if (c == ' ') {
          if (lww) return;
          else lww = true;
        } else lww = false;

        reducedValue += c;
      });

      // Removes the suffix and prefix whitespace from the value, this is just
      //  the whitespace at the end of line or the begin
      if (*(reducedValue.end() - 1) == ' ') reducedValue.pop_back();
      if (*reducedValue.begin() == ' ') reducedValue.erase(reducedValue.begin(), reducedValue.begin() + 1);

      // Builds the header and pushes it to the final result vector,
      //  which will join it back to a message
      string header = key;
      header += ':';
      header += reducedValue;
      result.push_back(make_pair(key, header));
    });

    // Sorts the results to order of the filter
    for_each(filter.begin(), filter.end(), [&](const string &key) {
      // Attempts to find the element if the key matches the key from
      //  the pair
      vector<pair<string, string>>::iterator element = find_if(result.begin(), result.end(), [&](pair<string, string> &pair) {
        return (pair.first == key);
      });

      // If there is no element, throw runtime error since this
      //  is strictly not allowed
      if (element == result.end())
        throw runtime_error(EXCEPT_DEBUG("Filter value not found in headers: '" + key + '\''));

      // Appends the header to the final result
      sortedResult.push_back(element->second);
    });

    if (sortedResult.size() <= 0) throw runtime_error(EXCEPT_DEBUG("Nothing left after canonicalizing"));
    return MIME::getStringFromLines(sortedResult.begin(), sortedResult.end());
  }
};
