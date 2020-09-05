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
  /**
   * Decodes an piece of text from an email
   */
  string decodeMIMEContent(
    strvec_it from, strvec_it to, const EmailTransferEncoding encoding
  ) {
    // Switches the transfer encoding, this will determine
    //  how we're going to process the current email
    switch (encoding) {
      // Decoedes 7bit/quoted printable, these use hex
      //  to encode special chars to make it 7bit
      case EmailTransferEncoding::ETE_QUOTED_PRINTABLE: {
        return Encoding::decodeQuotedPrintableRange(from, to);
      }

      // The default just keeps the original content in place
      //  most likely this happens for files.
      default:
      case EmailTransferEncoding::ETE_7BIT:
      case EmailTransferEncoding::ETE_8BIT:
      case EmailTransferEncoding::ETE_BASE64: {
        string result;

        for_each(from, to, [&](const string &line) {
          result += line + "\r\n";
        });

        return result;
      };
    }
  }

  /**
   * Gets the MIME body ranges from an message
   */
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

    return tuple<strvec_it, strvec_it, strvec_it, strvec_it>(from, headersEnd, bodyBegin, to);
  }

  /**
   * Joins MIME headers and parses them into EmailHeader's
   */
  vector<EmailHeader> parseHeaders(strvec_it from, strvec_it to) {
    _parseHeaders(from, to, false);
  }

  vector<EmailHeader> _parseHeaders(strvec_it from, strvec_it to, bool lowerKey) {
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
      if (sep == string::npos) {
        throw runtime_error(EXCEPT_DEBUG("Could not find k/v pair for header"));
      }

      auto key = a(header.substr(0, sep));

      // Turns the header key into lowercase if specified
      //  by the callee
      if (lowerKey) {
        transform(key.begin(), key.end(), key.begin(), [](const char c) {
          return tolower(c);
        });
      }

      // Pushes the header to the result vector
      headers.push_back(EmailHeader {
        key,
        a(header.substr(++sep))
      });
    });

    return headers;
  }

  /**
   * Splits header values using a semicolon
   */
  vector<string> splitHeaderBySemicolon(const string &raw) {
    vector<string> result = {};

    // Creates an stringstream which will be used
    //  to split the header by the semicolon
    stringstream stream(raw);
    string segment;
    while (getline(stream, segment, ';')) {
      result.push_back(segment);
    }

    return result;
  }

  /**
   * Parses key/value pairs from an email header value
   */
  map<string, string> parseHeaderKeyValuePairs(strvec_it from, strvec_it to) {
    map<string, string> result = {};
    
    for_each(from, to, [&](const string &segment) {
      size_t sep = segment.find_first_of('=');
      if (!sep) return;

      string key = segment.substr(0, sep);
      string value = segment.substr(++sep);

      if (*(key.end() - 1) == ' ') key.pop_back();
      if (*key.begin() == ' ') key.erase(key.begin());

      if (*(value.end() - 1) == ' ') value.pop_back();
      if (*value.begin() == ' ') value.erase(value.begin());
      if (*(value.end() - 1) == '"') value.pop_back();
      if (*value.begin() == '"') value.erase(value.begin());

      result.insert(make_pair(key, value));
    });

    return result;
  }

  /**
   * Gets the content type, boundary and charset from the content-type header
   */
  tuple<EmailContentType, string, string> parseContentType(const string &raw) {
    vector<string> segments = splitHeaderBySemicolon(raw);
    if (segments.size() <= 0) throw runtime_error(EXCEPT_DEBUG("Invalid content type"));

    // Gets the email content type, this is here in almost
    //  all cases since it is required    
    EmailContentType contentType = stringToEmailContentType(segments[0]);

    // Parses the other key/value pairs, and attempts to get the
    //  charset and the boundary, thse are both not required.
    string boundary, charset;
    
    auto valuePairs = parseHeaderKeyValuePairs(segments.begin()+1, segments.end());
    if (valuePairs.count("charset") > 0) charset = valuePairs.find("charset")->second;
    if (valuePairs.count("boundary") > 0) boundary = valuePairs.find("boundary")->second;

    return tuple<EmailContentType, string, string>(contentType, boundary, charset);
  }

  /**
   * Gets the default set of email information from an mime message
   */
  void parseMIMEDataFromHeaders(
    const string &key, const string &value, FullEmail &email
  ) {
    // Checks the key, and based on that decides what to do
    //  with the key/value pair
    if (key == "subject") email.e_Subject = value;
    else if (key == "message-id") email.e_MessageID = value;
    else if (key == "date") {
      struct tm t;
      if (strptime(value.c_str(), "%a, %d %b %Y %T %Z", &t) == nullptr) {
        throw runtime_error(EXCEPT_DEBUG("Invalid date format"));
      }
      email.e_Date = timegm(&t);
    } else if (key == "from") email.e_From = EmailAddress::parseAddressList(value);
    else if (key == "to") email.e_To = EmailAddress::parseAddressList(value);
  }

  /**
   * Gets the section ranges of an multipart body separated by an boundary
   */
  vector<tuple<strvec_it, strvec_it>> splitMultipartMessage(strvec_it from, strvec_it to, const string &boundary) {
    vector<tuple<strvec_it, strvec_it>> result = {};

    const string &sectionStartBoundary = "--" + boundary;
    const string sectionEndBoundary = "--" + boundary + "--";

    // Splits the body into sections using the boundaries
    //  when the end boundary is hit we close the loop
    size_t i = 0;
    strvec_it prev = from;
    for (strvec_it it = from; it != to; ++it) {
      auto &line = *it;
      if (line[0] == '-' && line[1] == '-') {
        // Checks if an boundary is hit, if so
        //  set a 8 bit value to either 1 ( section )
        //  or 2 ( end )
        uint8_t boundaryHit = 0;
        if (line == sectionStartBoundary) boundaryHit = 1;
        else if (line == sectionEndBoundary) boundaryHit = 2;

        // Checks which action to perform, if section or end is
        //  hit we will append the current section, after that we
        //  check if the end boundary is hit, if so break
        if (boundaryHit == 2 || boundaryHit == 1) {
          if (i++ > 0) {
            result.push_back(make_pair(++prev, it));
            prev = it;
          }
        }
        
        if (boundaryHit == 2) break;
      }
    }

    // If SMTP_DEBUG set perform the debug print of
    //  the parsed sections
    // #ifdef _SMTP_DEBUG
    // Logger logger("MIMEV2", LoggerLevel::DEBUG);

    // for_each(result.begin(), result.end(), [&](const tuple<strvec_it, strvec_it> &tup) {
    //   logger << "Parsed section: '" << ENDL;

    //   strvec_it start, end;
    //   tie(start, end) = tup;
    //   for_each(start, end, [](const string &l) {
    //     cout << l << endl;
    //   });

    //   cout << '\'' << endl;
    // });
    // #endif

    return result;
  }

  /**
   * Performs a recursive round on a mime message, each round gets an iterator
   *  range on which the parsing will be performed
   */
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

    EmailContentType contentType;
    EmailTransferEncoding contentTransferEncoding;
    string boundary, charset;

    // Parses the headers into an vector, if we're in the
    //  first round store the headers inside of the email
    vector<EmailHeader> headers = {};
    headers = parseHeaders(headersBegin, headersEnd);
    if (!i) email.e_Headers = headers;

    // Prints the found headers, only if
    //  debug mode is enabled
    #ifdef _SMTP_DEBUG
    logger << "Found headers: " << ENDL;
    
    size_t j = 0;
    for_each(headers.begin(), headers.end(), [&](const EmailHeader &h) {
      logger << j++ << " -> '" << h.e_Key << "': '" << h.e_Value << '\'' << ENDL;
    });
    #endif

    // Gets the default fields, such as the date
    //  boundary, subject etcetera
    for_each(headers.begin(), headers.end(), [&](EmailHeader &h) {
      auto &key = h.e_Key;
      auto &value = h.e_Value;

      // Turns the key into lowercase for easier comparison,
      //  this will be also visible in the result since it modifies
      //  a reference
      transform(key.begin(), key.end(), key.begin(), [](const char c) {
        return tolower(c);
      });

      // Checks if we're in the first iteration, if so
      //  fetch the basic data
      if (!i) parseMIMEDataFromHeaders(key, value, email);
      if (key == "content-type") {
        tie(contentType, boundary, charset) = parseContentType(value);
      } else if (key == "content-transfer-encoding") {
        contentTransferEncoding = stringToEmailTransferEncoding(value);
        cout << contentTransferEncodingToString(contentTransferEncoding) << endl;
      }
    });

    // ================================
    // Starts parsing the body / body
    //  sections, based on the content
    //  type
    // ================================

    DEBUG_ONLY(logger << "Parsing section of type: " << contentTypeToString(contentType) << ENDL);
    switch (contentType) {
      case EmailContentType::ECT_MULTIPART_ALTERNATIVE:
      case EmailContentType::ECT_MULTIPART_MIXED: {
        // Performs the recursive parsing of all the sections,
        //  this will be done by the same function
        for (tuple<strvec_it, strvec_it> sec : splitMultipartMessage(bodyBegin, bodyEnd, boundary)) {
          parseMIMERecursive(email, i+1, get<0>(sec), get<1>(sec));
        }
        break;
      }
      case EmailContentType::ECT_TEXT_PLAIN:
      case EmailContentType::ECT_TEXT_HTML:
      default: {
        email.e_BodySections.push_back(EmailBodySection {
          decodeMIMEContent(bodyBegin, bodyEnd, contentTransferEncoding),
          contentType, headers, i, contentTransferEncoding
        });
      }
    }
  }

  /**
   * Parses an raw MIME Message into an valid FullEmail
   */
  void parseMIME(const string &raw, FullEmail &email) {
    #ifdef _SMTP_DEBUG
    Logger logger("MIMEV2", LoggerLevel::DEBUG);
    Timer timer("parseMIME()", logger);
    #endif

    // Splits the message into lines, which later can be used
    //  to iterate over in the different rounds
    vector<string> lines = getMIMELines(raw);

    parseMIMERecursive(email, 0, lines.begin(), lines.end());
  }

  /**
   * Turns an message into a vector of lines
   */
  vector<string> getMIMELines(const string &raw) {
    vector<string> lines = {};

    stringstream stream(raw);
    string line;
    while (getline(stream, line, '\n')) {
      if (line[line.length() - 1] == '\r') line.pop_back();
      lines.push_back(line);
    }

    return lines;
  }

  /**
   * Turns the lines into a string
   */
  string getStringFromLines(strvec_it from, strvec_it to) {
    string result;

    for_each(from, to, [&](const string &line) {
      result += line + "\r\n";
    });

    return result;  
  }
}
