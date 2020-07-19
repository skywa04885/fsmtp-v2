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

#include "P3.src.h"

namespace FSMTP::POP3
{
	typedef enum : uint8_t
	{
		PCT_CAPA = 0,
		PCT_QUIT,
		PCT_STLS,
		PCT_USER,
		PCT_PASS,
		PCT_UNKNOWN,
		PCT_STAT,
		PCT_UIDL,
		PCT_LIST,
		PCT_RETR,
		PCT_DELE,
		PCT_TOP,
		PCT_RSET,
		PCT_IMPLEMENTATION
	} POP3CommandType;

	class P3Command
	{
	public:
		void parse(const std::string &raw);

		POP3CommandType c_Type;
		std::vector<std::string> c_Args;
	};
}
