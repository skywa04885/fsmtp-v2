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

#include "DKIMValidator.src.h"

namespace FSMTP::DKIM {
  DKIMValidator::DKIMValidator():
    m_Logger("DKIMValidator", LoggerLevel::DEBUG)
  {
    this->m_Result.type = DKIMValidatorResultType::DKIMValidationFail;
  }

  DKIMSignatureResult DKIMValidator::validateSignature(
    const string &signature, 
    const vector<EmailHeader> &headers, const string &body,
    const string &rawHeaders
  ) {
    auto &logger = this->m_Logger;

    // =================================
    // Parses the header
    // =================================

    // Parses the signature, so we can get the values from it
    //  and process it accordingly
    DKIMHeader header;
    header.parse(signature);
    DEBUG_ONLY(header.print(logger));

    // =================================
    // Canonicalizes the body
    // =================================

    // Canonicalizes the body
    string canonicalizedBody;
    switch (header.getCanonAlgorithmPair()) {
      case DKIMHeaderCanonAlgPair::RelaxedRelaxed:
      case DKIMHeaderCanonAlgPair::SimpleRelaxed:
        DEBUG_ONLY(logger << "Processing body with relaxed body canonicalization" << ENDL);
        canonicalizedBody = relaxedBody(body);
        break;
      case DKIMHeaderCanonAlgPair::RelaxedSimple:
      case DKIMHeaderCanonAlgPair::SimpleSimple:
        DEBUG_ONLY(logger << "Processing body with simple body canonicalization" << ENDL);
        canonicalizedBody = simpleBody(body);
        break;
    }

    // Calculates the hash value, either sha1 or sha256, after which we compare it
    //  against the one in the message, if the comparison fails, do not even check the
    //  signature, just return error
    string bodyHash;
    switch (header.getHeaderAlgorithm()) {
      case DKIMHeaderAlgorithm::HeaderAlgoritmRSA_SHA256:
        bodyHash = Hashes::sha256base64(canonicalizedBody);
        break;
      case DKIMHeaderAlgorithm::HeaderAlgorithmRSA_SHA1:
        bodyHash = Hashes::sha1base64(canonicalizedBody);
        break;
    }

    if (bodyHash != header.getBodyHash()) {
      DEBUG_ONLY(logger << WARN << "Body hash invalid, expected: '" << bodyHash << "', got: '" << header.getBodyHash() << '\'' << ENDL << CLASSIC);
      return DKIMSignatureResult {
        DKIMSignatureResultType::DKIMSignatureInvalid,
        "body-hash invalid, expected: " + bodyHash
      };
    } 

    DEBUG_ONLY(logger << "Body hash valid, expected: '" << bodyHash << "', got: '" << header.getBodyHash() << "'" << ENDL);


    // =================================
    // Gets the DKIM record from DNS
    // =================================
    
    // Creates the query, and resolves the record from DNS,
    //  if this fails return no record found
    string query = header.getKeySelector();
    query += "._domainkey." + header.getDomain();
    DEBUG_ONLY(logger << "Resolving DKIM for: '" << query << '\'' << ENDL);
    
    DKIMRecord record;
    try {
      record = DKIMRecord::fromDNS(query.c_str());
      DEBUG_ONLY(record.print(logger));
    } catch (const runtime_error &e) {
      DEBUG_ONLY(logger << ERROR << "Could not find record for query: '" << query << '\'' << ENDL << CLASSIC);
      return DKIMSignatureResult {
        DKIMSignatureResultType::DKIMSignatureRecordNotFound,
        "No record found for query: '" + query + '\''
      };
    }

    // =================================
    // Generates canonicalized headers
    // =================================

    // Gets the index of the signature itself inside of the raw
    //  signature header, after which we get the substring of it
    size_t signatureIndex = signature.find("b=");
    if (signatureIndex == string::npos) {
      DEBUG_ONLY(logger << ERROR << "Could not find signature index in header" << ENDL << CLASSIC);
      return DKIMSignatureResult {
        DKIMSignatureResultType::DKIMSignatureInvalid,
        "No signature found in the DKIM-Signature header"
      };
    }

    // Puts the raw headers into an string after which we will append
    //  the pre-sign signature
    string precanonicalizeHeaders = rawHeaders;
    precanonicalizeHeaders += "DKIM-Signature: " + signature.substr(0, signatureIndex + 2) + "\r\n";

    // Checks which algorithm to use and canonicalizes the headers
    //  with it
    vector<string> headerFilter = header.getHeaders();
    headerFilter.push_back("dkim-signature");
    
    string canonicalizedHeaders;
    switch (header.getCanonAlgorithmPair()) {
      case DKIMHeaderCanonAlgPair::RelaxedRelaxed:
      case DKIMHeaderCanonAlgPair::RelaxedSimple:
        DEBUG_ONLY(logger << "Processing headers with relaxed canonicalization" << ENDL);
        canonicalizedHeaders = relaxedHeaders(precanonicalizeHeaders, headerFilter);
        break;
      case DKIMHeaderCanonAlgPair::SimpleRelaxed:
      case DKIMHeaderCanonAlgPair::SimpleSimple:
        DEBUG_ONLY(logger << "Processing headers with simple canonicalization" << ENDL);
        canonicalizedHeaders = simpleHeaders(precanonicalizeHeaders, headerFilter);
        break;
    }

    // Removes the last CRLF from the canonicalized headers, since the last of the headers
    //  is the dkim signature without a signature, so not the end
    canonicalizedHeaders.erase(canonicalizedHeaders.end() - 2, canonicalizedHeaders.end());

    cout << '\'' << canonicalizedHeaders << '\'' << endl;

    // =================================
    // Validates the signature
    // =================================

    switch (header.getHeaderAlgorithm()) {
      case DKIMHeaderAlgorithm::HeaderAlgorithmRSA_SHA1:
        if (!Hashes::RSAverify(header.getSignature(), canonicalizedHeaders, record.getPublicKey(), EVP_sha1())) {
          DEBUG_ONLY(logger << WARN << "RSA-SHA1 Signature is invalid" << ENDL << CLASSIC);
          return DKIMSignatureResult {
            DKIMSignatureResultType::DKIMSignatureInvalid,
            "RSA-SHA1 Signature invalid"
          };
        }

        DEBUG_ONLY(logger << "RSA-SHA1 Signature is valid !" << ENDL << CLASSIC);
        break;
      case DKIMHeaderAlgorithm::HeaderAlgoritmRSA_SHA256:
        if (!Hashes::RSAverify(header.getSignature(), canonicalizedHeaders, record.getPublicKey(), EVP_sha256())) {
          DEBUG_ONLY(logger << WARN << "RSA-SHA256 Signature is invalid" << ENDL << CLASSIC);
          return DKIMSignatureResult {
            DKIMSignatureResultType::DKIMSignatureInvalid,
            "RSA-SHA256 Signature invalid"
          };
        }

        DEBUG_ONLY(logger << "RSA-SHA256 Signature is valid !" << ENDL << CLASSIC);
        break;
    }

    return DKIMSignatureResult {
      DKIMSignatureResultType::DKIMSignatureValid,
      "signature and body-hash are valid"
    };
  }

  DKIMValidator &DKIMValidator::validate(const string &message) {
    auto &logger = this->m_Logger;

    #ifdef _SMTP_DEBUG
    logger << "Started validating message of size: " << message.size() << " bytes" << ENDL;
    Timer t("Validate", logger);
    #endif
    
    // =================================
    // Parses the MIME message
    // =================================

    // Parses the message into lines, after which we get the body and headers
    //  so we can process them later
    vector<string> lines = Parsers::getMIMELines(message);
    strvec_it headersBegin, headersEnd, bodyBegin, bodyEnd;
    tie(headersBegin, headersEnd, bodyBegin, bodyEnd) = Parsers::splitMIMEBodyAndHeaders(lines.begin(), lines.end());

    // Parses the headers into an vector of email key/value pairs
    vector<EmailHeader> headers = Parsers::_parseHeaders(headersBegin, headersEnd, true);

    // Gets the body string, since we will have to canonicalize it 
    //  later on in the process
    string rawBody = Parsers::getStringFromLines(bodyBegin, bodyEnd);

    // Gets the raw headers of the message, we will remove the DKIM-Signature ones from
    //  it since it will otherwise confuse the validation process
    vector<string> rawHeadersWithoutSignatures = {};
    for_each(headers.begin(), headers.end(), [&](const EmailHeader &header) {
      if (header.e_Key == "dkim-signature") return;
      else rawHeadersWithoutSignatures.push_back(header.e_Key + ": " + header.e_Value);
    });
    string rawHeaders = Parsers::getStringFromLines(rawHeadersWithoutSignatures.begin(), rawHeadersWithoutSignatures.end());

    // =================================
    // Gets the signatures
    // =================================

    // Gets the string references to the signatures
    vector<string> signatures = {};
    for_each(headers.begin(), headers.end(), [&](const EmailHeader &h) {
      if (h.e_Key == "dkim-signature")
        signatures.push_back(h.e_Value);
    });

    if (signatures.size() <= 0) {
      DEBUG_ONLY(logger << WARN << "Zero signatures in message, throwing neutral ..." << ENDL << CLASSIC);
      this->m_Result.type = DKIMValidatorResultType::DKIMValidationNeutral;
      this->m_Result.details = "No DKIM-Signatures found in headers";
      return *this;
    }

    // =================================
    // Starts validating the signatures
    // =================================

    // Loops over the signatures, and validates each one. If an exception is thrown
    //  we catch it, and put an system failure as result, will result in neutral
    for_each(signatures.begin(), signatures.end(), [&](const string &sig) {
      try { this->m_SigResults.push_back(this->validateSignature(sig, headers, rawBody, rawHeaders)); }
      catch (const runtime_error &e) {
        DEBUG_ONLY(logger << ERROR << "Signature validation system failure: " << e.what() << ENDL << CLASSIC);
        this->m_SigResults.push_back(DKIMSignatureResult {
          DKIMSignatureResultType::DKIMSignatureSystemFailure,
          "System failure, check logs"
        });
      } catch (...) {
        DEBUG_ONLY(logger << ERROR << "Signature validation system failure: unknown" << ENDL << CLASSIC);
        this->m_SigResults.push_back(DKIMSignatureResult {
          DKIMSignatureResultType::DKIMSignatureSystemFailure,
          "System failure, error unknown"
        });
      }
    });
    
    // Checks if one of the signatures is valid, so we can mark the message as valid
    //  or not valid
    any_of(this->m_SigResults.begin(), this->m_SigResults.end(), [&](const DKIMSignatureResult &res) {
      if (res.type == DKIMSignatureResultType::DKIMSignatureValid) {
        this->m_Result.type = DKIMValidatorResultType::DKIMValidationPass;
        return false;
      } else return true;
    });
    return *this;
  }

  const DKIMValidatorResult &DKIMValidator::getResult() {
    return this->m_Result;
  }

  string DKIMValidator::getResultString() {
    string result;

    switch (this->m_Result.type) {
      case DKIMValidatorResultType::DKIMValidationPass:
        result += "pass (";
        break;
      case DKIMValidatorResultType::DKIMValidationNeutral:
        result += "neutral (";
        break;
      case DKIMValidatorResultType::DKIMValidationSystemError:
        result += "error (";
        break;
      case DKIMValidatorResultType::DKIMValidationFail: 
        result += "fail (";
        break;
    }

    if (this->m_SigResults.size() <= 0) result += this->m_Result.details;
    else {
      size_t i = 0;
      for_each(this->m_SigResults.begin(), this->m_SigResults.end(), [&](const DKIMSignatureResult &res) {
        result += "signature " + to_string(++i) + ": " + res.details;
        if (i < this->m_SigResults.size()) result += ", ";
      });
    }

    result += ')';

    return result;
  }

  DKIMValidator::~DKIMValidator() = default;
}