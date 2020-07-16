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

#include "P3Response.src.h"

namespace FSMTP::POP3
{
	P3Response::P3Response(const bool p_Ok, const POP3ResponseType p_Type):
		p_Type(p_Type), p_Ok(p_Ok)
	{}

	std::string P3Response::build(void)
	{
		std::string res;

		// Adds ok or failure, and then the message
		if (!p_Ok) res += "-ERR ";
		else res += "+OK ";
		res += this->getMessage();

		// Adds the CRLF
		res += "\r\n";

		return res;
	}

	std::string P3Response::getMessage(void)
	{
		switch (this->p_Type)
		{
			case POP3ResponseType::PRT_GREETING:
			{
				std::string ret = "POP3 server ready <";
				ret += _SMTP_SERVICE_NODE_NAME;
				ret += ">";
				return ret;
			}
		}
	}
}