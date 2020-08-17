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

#include "Email.src.h"


namespace FSMTP::Models
{
	EmailAddress::EmailAddress() {}

	EmailAddress::EmailAddress(
		const string &e_Name,
		const string &e_Address
	):
		e_Name(e_Name), e_Address(e_Address)
	{}

	EmailAddress::EmailAddress(const string &raw) {
		this->parse(raw);
	}

  string EmailAddress::toString() const {
		ostringstream oss;

		if (!this->e_Name.empty()) {
			oss << this->e_Name << ' ';
		}
    oss << "<" << this->e_Address << ">";
    
    return oss.str();
  }

	void EmailAddress::parse(const string &raw) {

    // Validates the raw address before proceeding with further parsing,
    //  this is to detect an error early, and safe computational power

		if (raw.find_first_of('@') == string::npos) {
			throw runtime_error("Could not find @");
    } else if (raw.find_last_of('.') == string::npos) {
      throw runtime_error("Could not find domain extension");
    }

    // Checks if we're dealing with an address only, or an name + address
    //  these both will be processed different. If we end up with an name
    //  we also remove the duplicate whitespace and the quotes from the name

		size_t openBracket = raw.find_first_of('<'), closeBracket = raw.find_first_of('>');
		if (openBracket != string::npos && closeBracket != string::npos) {
			this->e_Address = raw.substr(openBracket + 1, closeBracket - openBracket - 1);
			this->e_Name = raw.substr(0, openBracket);

			string clean;
			reduceWhitespace(this->e_Name, clean);
			removeFirstAndLastWhite(clean);
			removeStringQuotes(clean);
			this->e_Name = clean;
		} else if (
			(openBracket == string::npos && closeBracket != string::npos) ||
			(openBracket != string::npos && closeBracket == string::npos)
		) {
			throw runtime_error("Only one bracket found, address should contain both opening and closing.");
		} else {
			this->e_Address = raw;
		}

		removeFirstAndLastWhite(this->e_Address);
	}

  vector<EmailAddress> EmailAddress::parseAddressList(const string &raw) {
    stringstream stream(raw);
    string token;
    vector<EmailAddress> result = {};

    while (getline(stream, token, ',')) {
      result.push_back(EmailAddress(token));
    }

    return result;
  }

  string EmailAddress::addressListToString(const vector<EmailAddress> &addresses) {
    ostringstream oss;
    size_t i = 0;

    for (const EmailAddress &a : addresses) {
      if (!a.e_Name.empty()) {
        oss << '"' << a.e_Name << "\" ";
      }

      oss << '<' << a.e_Address << '>';
      
      if (++i < addresses.size()) {
        oss << ", ";
      }
    }

    return oss.str();
  }

	string EmailAddress::getDomain() const {
    auto &addr = this->e_Address;

		size_t index = addr.find_first_of('@');
		if (index == string::npos) {
			throw runtime_error("Invalid email address");
    }
		return addr.substr(index + 1);
	}

	string EmailAddress::getUsername() const {
    auto &addr = this->e_Address;

		size_t index = addr.find_first_of('@');
		if (index == string::npos) {
			throw runtime_error("Invalid email address");
    }
		return addr.substr(0, index);
	}


	// ======================================================
	// The email stuff, instead of the address stuff
	// ======================================================
  
  FullEmail::FullEmail() {}

	EmailContentType stringToEmailContentType(const string &raw) {
		if (raw == "multipart/alternative") return EmailContentType::ECT_MULTIPART_ALTERNATIVE;
		else if (raw == "multipart/mixed") return EmailContentType::ECT_MULTIPART_MIXED;
		else if (raw == "text/plain") return EmailContentType::ECT_TEXT_PLAIN;
		else if (raw == "text/html") return EmailContentType::ECT_TEXT_HTML;
		else return EmailContentType::ECT_NOT_FUCKING_KNOWN;
	}

  const char *contentTypeToString(const EmailContentType type) {
    switch (type)
    {
      case EmailContentType::ECT_TEXT_PLAIN: return "text/plain";
      case EmailContentType::ECT_TEXT_HTML: return "text/html";
      case EmailContentType::ECT_MULTIPART_ALTERNATIVE: return "multipart/alternative";
      case EmailContentType::ECT_MULTIPART_MIXED: return "multipart/mixed";
      default: return "application/octet-stream";
    }
  }

	EmailTransferEncoding stringToEmailTransferEncoding(const string &raw) {
		if (raw == "7bit") return EmailTransferEncoding::ETE_7BIT;
		else if (raw == "8bit") return EmailTransferEncoding::ETE_8BIT;
		else if (raw == "base64") return EmailTransferEncoding::ETE_QUOTED_PRINTABLE;
    else if (raw == "quoted-printable") return EmailTransferEncoding::ETE_BASE64;
		else return EmailTransferEncoding::ETE_NOT_FUCKING_KNOWN;
	}

  const char *contentTransferEncodingToString(const EmailTransferEncoding enc) {
    switch (enc) {
      case EmailTransferEncoding::ETE_8BIT: return "8bit";
      case EmailTransferEncoding::ETE_BASE64: return "base64";
      case EmailTransferEncoding::ETE_7BIT: return "7bit";
      case EmailTransferEncoding::ETE_QUOTED_PRINTABLE: return "quoted-printable";
      default: return "text/plain";
    }
  }

  void FullEmail::print(FullEmail &email, Logger &logger) {
  	logger << DEBUG;
    DEFER(logger << CLASSIC);

  	// ========================================
  	// Prints the basic email data
  	// ========================================

  	logger << "FullEmail:" << ENDL;
  	logger << " - Transport From: " << email.e_TransportFrom.e_Name << '<' << email.e_TransportFrom.e_Address << '>' << ENDL;
  	logger << " - Transport To: " << ENDL;
		for_each(email.e_TransportTo.begin(), email.e_TransportTo.end(), [&](auto &to) {
			logger << "\t - \"" << to.e_Name << "\" <" << to.e_Address << '>' << ENDL;
		});
  	logger << " - Subject: " << email.e_Subject << ENDL;
  	logger << " - Date: " << email.e_Date << ENDL;
  	logger << " - Message ID: " << email.e_MessageID << ENDL;
  	logger << " - Headers (Without Microsoft Bullshit): " << ENDL;

  	// ========================================
  	// Prints the arrays
  	// ========================================
  	
  	size_t c = 0;
  	for_each(email.e_Headers.begin(), email.e_Headers.end(), [&](auto &h){
  		logger << "\t - Header[no: " << c++ << "]: <";
  		logger << h.e_Key << "> - <" << h.e_Value << ">" << ENDL;
  	});

    c = 0;
  	logger << " - Body sections: " << ENDL;
  	for_each(email.e_BodySections.begin(), email.e_BodySections.end(), [&](auto &s) {
  		logger << "\t - Body Section[no: " << c++ << ", cType: ";
  		logger << s.e_Type << ", cTransEnc: "; 
  		logger << s.e_TransferEncoding << "]: " << ENDL;
  		logger << "\t\t" << (s.e_Content.size() < 56 ? s.e_Content : s.e_Content.substr(0, 56) + " ...") << ENDL;
  	});

    auto printAddressList = [&](const char *label, const vector<EmailAddress> &vec) {
      c = 0;
      logger << " - " << label << ": " << ENDL;
      for_each(vec.begin(), vec.end(), [&](auto &address) {
        logger << "\t - Email[no: " << c++ << "]: ";
        logger << address.e_Name << " | " << address.e_Address << ENDL;
      });
    };

    printAddressList("From", email.e_From);
    printAddressList("To", email.e_To);
  }

  int64_t FullEmail::getBucket(void) {
  	int64_t now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
  	return now / 1000 / 1000 / 10;
  }

  CassUuid FullEmail::generateMessageUUID(void) {
		CassUuid uuid;
    CassUuidGen *gen = cass_uuid_gen_new();
    DEFER(cass_uuid_gen_free(gen));

    cass_uuid_gen_time(gen, &uuid);
		return uuid;
  }
}
