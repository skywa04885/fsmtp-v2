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

	const std::string &EmailAddress::getAddress(void)
	{
		return this->e_Address;
	}

	const std::string &EmailAddress::getName(void)
	{
		return this->e_Name;
	}

  // The Email class stuff
  
  FullEmail::FullEmail()
  {}
}
