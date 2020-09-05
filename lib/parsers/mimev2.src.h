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

#ifndef _SRC_PARSERS_MIME2_H
#define _SRC_PARSERS_MIME2_H

#include "../default.h"

#include "../general/Logger.src.h"
#include "../general/Timer.src.h"
#include "../general/encoding.src.h"
#include "../models/Email.src.h"

using namespace FSMTP::Models;

namespace FSMTP::Parsers {
  /**
   * Decodes an piece of text from an email
   */
  string decodeMIMEContent(
    strvec_it from, strvec_it to, const EmailTransferEncoding encoding
  );
  
  /**
   * Gets the MIME body ranges from an message
   */
  tuple<strvec_it, strvec_it, strvec_it, strvec_it> splitMIMEBodyAndHeaders(strvec_it from, strvec_it to);

  /**
   * Joins MIME headers and parses them into EmailHeader's
   */
  vector<EmailHeader> parseHeaders(strvec_it from, strvec_it to);
  vector<EmailHeader> _parseHeaders(strvec_it from, strvec_it to, bool lowerKey);  
  
  /**
   * Splits header values using a semicolon
   */
  vector<string> splitHeaderBySemicolon(const string &raw);
  
  /**
   * Parses key/value pairs from an email header value
   */
  map<string, string> parseHeaderKeyValuePairs(strvec_it from, strvec_it to);

  /**
   * Gets the content type, boundary and charset from the content-type header
   */
  tuple<EmailContentType, string, string> parseContentType(const string &raw);
  
  /**
   * Gets the default set of email information from an mime message
   */
  void parseMIMEDataFromHeaders(
    const string &key, const string &value, FullEmail &email
  );

  /**
   * Gets the section ranges of an multipart body separated by an boundary
   */
  vector<tuple<strvec_it, strvec_it>> splitMultipartMessage(strvec_it from, strvec_it to, const string &boundary);

  /**
   * Performs a recursive round on a mime message, each round gets an iterator
   *  range on which the parsing will be performed
   */
  void parseMIMERecursive(
    FullEmail &email, const size_t i, strvec_it from, strvec_it to
  );

  /**
   * Parses an raw MIME Message into an valid FullEmail
   */
  void parseMIME(const string &raw, FullEmail &email);

  /**
   * Turns an message into a vector of lines
   */
  vector<string> getMIMELines(const string &raw);

  /**
   * Turns the lines into a string
   */
  string getStringFromLines(strvec_it from, strvec_it to);
};

#endif