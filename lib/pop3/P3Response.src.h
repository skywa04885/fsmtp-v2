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
		PRT_GREETING = 0
	} POP3ResponseType;

	class P3Response
	{
	public:
		P3Response(const bool ok, const POP3ResponseType p_Type);

		std::string build(void);

		std::string getMessage(void);
	private:
		POP3ResponseType p_Type;
		bool p_Ok;
		std::string p_Message;
	};
}