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

#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <cstdint>

#include "../general/cleanup.src.h"

using namespace FSMTP::Cleanup;

namespace FSMTP::Models
{
	class EmailAddress
	{
	public:
		EmailAddress(const std::string &e_Name, const std::string &e_Address);
		EmailAddress(const std::string &raw);
		EmailAddress();

		void parse(const std::string &raw);

		const std::string &getAddress(void);
		const std::string &getName(void);

		void setAddress(const std::string &address);
		void setName(const std::string &name);

		void getDomain(std::string &ret);
		void getUsername(std::string &ret);
	private:
		std::string e_Address;
		std::string e_Name;
	};

	class EmailHeader
	{
	public:
	private:
		std::string e_Key;
		std::string e_Value;
	};

  typedef enum : uint8_t
  {

  } FullEmailType;

	class FullEmail
	{
	public:
    FullEmail();

    FullEmailType e_Type;
    EmailAddress e_TransportFrom;
		EmailAddress e_TransportTo;
		std::string e_Subject;
		std::vector<EmailAddress> e_From;
		std::vector<EmailAddress> e_To;
		std::vector<EmailHeader> e_Headers;
	};

}
