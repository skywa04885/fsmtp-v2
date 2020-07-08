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

#include "email.src.h"


namespace FSMTP::Models
{
	// ======================================================
	// The email address stuff
	// ======================================================

	EmailAddress::EmailAddress(
		const std::string &e_Name,
		const std::string &e_Address
	):
		e_Name(e_Name), e_Address(e_Address)
	{}

	EmailAddress::EmailAddress(const std::string &raw)
	{
		this->parse(raw);
	}

	EmailAddress::EmailAddress():
		e_Name(),
		e_Address()
	{

	}

	void EmailAddress::parse(const std::string &raw)
	{
		// Checks the email format type, since there may
		// - be multiple formats, and we need to parse it
		// - in the correct way
		std::size_t openBracket = raw.find_first_of('<'), closeBracket = raw.find_first_of('>');
		if (openBracket != std::string::npos && closeBracket != std::string::npos)
		{
			this->e_Address = raw.substr(openBracket + 1, closeBracket - openBracket - 1);
			this->e_Name = raw.substr(0, openBracket);
			removeFirstAndLastWhite(this->e_Name);
		} else if (
			openBracket == std::string::npos && closeBracket != std::string::npos ||
			openBracket != std::string::npos && closeBracket == std::string::npos
		)
			throw std::runtime_error("Only one bracket found, address "
				"should contain both opening and closing.");
		else this->e_Address = raw;

		// Validates the email address by checking for the at symbol
		if (raw.find_first_of('@') == std::string::npos)
			throw std::runtime_error("Invalid address");

		// Removes the non-required whitespace, this is always required
		// - so used as last, while the name whitespace is only required when
		// - the name was parsed
		removeFirstAndLastWhite(this->e_Address);
	}

	void EmailAddress::getDomain(std::string &ret)
	{
		std::size_t index = this->e_Address.find_first_of('@');
		if (index == std::string::npos)
			throw std::runtime_error("Invalid email address");
		ret = this->e_Address.substr(index + 1);
	}

	void EmailAddress::getUsername(std::string &ret)
	{
		std::size_t index = this->e_Address.find_first_of('@');
		if (index == std::string::npos)
			throw std::runtime_error("Invalid email address");
		ret = this->e_Address.substr(0, index);
	}

	// ======================================================
	// The email stuff, instead of the address stuff
	// ======================================================
  
  FullEmail::FullEmail()
  {}

  /**
	 * Turns an string into an enum value
	 * - of email content type
	 *
	 * @Param {const std::string &} raw
	 * @Return {EmailContentType}
	 */
	EmailContentType stringToEmailContentType(const std::string &raw)
	{
		if (raw == "multipart/alternative") return EmailContentType::ECT_MULTIPART_ALTERNATIVE;
		else if (raw == "multipart/mixed") return EmailContentType::ECT_MULTIPART_MIXED;
		else if (raw == "text/plain") return EmailContentType::ECT_TEXT_PLAIN;
		else if (raw == "text/html") return EmailContentType::ECT_TEXT_HTML;
		else return EmailContentType::ECT_NOT_FUCKING_KNOWN;
	}

	/**
	 * Turns an string into an enum value of
	 * - EmailTransferEncoding type
	 *
	 * @Param {const std::string &} raw
	 * @Return {EmailContentType}
	 */
	EmailTransferEncoding stringToEmailTransferEncoding(const std::string &raw)
	{
		if (raw == "7bit") return EmailTransferEncoding::ETE_7BIT;
		else if (raw == "8bit") return EmailTransferEncoding::ETE_8BIT;
		else return EmailTransferEncoding::ETE_NOT_FUCKING_KNOWN;
	}


  /**
   * Prints an full email to the console
   *
   * @Param {FullEmail &} email
   * @Param {Logger &logger} logger
   * @Return {void}
   */
  void FullEmail::print(FullEmail &email, Logger &logger)
  {
  	logger << DEBUG;

  	// Prints the basic email information
  	logger << "FullEmail:" << ENDL;
  	logger << " - Transport From: " << email.e_TransportFrom.e_Name << '<' << email.e_TransportFrom.e_Address << '>' << ENDL;
  	logger << " - Transport To: " << email.e_TransportTo.e_Name << '<' << email.e_TransportTo.e_Address << '>' << ENDL;
  	logger << " - Subject: " << email.e_Subject << ENDL;
  	logger << " - Content Type: " << email.e_ContentType << ENDL;
  	logger << " - Message ID: " << email.e_MessageID << ENDL;
  	logger << " - Headers (Without Microsoft Bullshit): " << ENDL;

  	std::size_t c = 0;
  	for (const EmailHeader &h : email.e_Headers)
  	{
  		logger << "\t - Header[no: " << c++ << "]: <"
  			<< h.e_Key << "> - <" << h.e_Value << ">" << ENDL;
  	}

  	logger << " - Body sections: " << ENDL;
  	c = 0;
  	for (const EmailBodySection &s : email.e_BodySections)
  	{
  		logger << "\t - Body Section[no: " << c++ << ", cType: "
  			<< s.e_Type << ", cTransEnc: " 
  			<< s.e_TransferEncoding << "]: " << ENDL;

  		logger << "\t\t" << s.e_Content << ENDL;
  	}

  	logger << CLASSIC;
  }
}
