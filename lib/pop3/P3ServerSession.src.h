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

#include <cstdint>
#include <string>

#include "P3.src.h"

using namespace FSMTP::Models;

namespace FSMTP::POP3
{
	class P3ServerSession
	{
	public:
		P3ServerSession();

		std::string s_User;
		std::string s_Pass;
		AccountShortcut s_Account;
	private:
		int64_t s_Flags;
		int64_t s_Actions;
	};
}
