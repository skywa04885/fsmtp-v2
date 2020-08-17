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

using namespace FSMTP::Models;

#define _P3_SERVER_SESS_ACTION_USER 1
#define _P3_SERVER_SESS_ACTION_PASS 2

#define _P3_SERVER_SESS_FLAG_AUTH 1

namespace FSMTP::POP3
{
	class P3ServerSession
	{
	public:
		P3ServerSession();

		void setFlag(int64_t mask);
		bool getFlag(int64_t mask);

		void setAction(int64_t mask);
		bool getAction(int64_t mask);

		string s_User;
		string s_Pass;
		AccountShortcut s_Account;
		vector<tuple<CassUuid, int64_t, int64_t>> s_References;
		vector<size_t> s_Graveyard;
	private:
		int64_t s_Flags;
		int64_t s_Actions;
	};
}
