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

#define _P3_SERVER_SESS_ACTION_USER 1
#define _P3_SERVER_SESS_ACTION_PASS 2

#define _P3_SERVER_SESS_FLAG_AUTH 1

namespace FSMTP::POP3
{
	class P3ServerSession
	{
	public:
		/**
		 * Initializes the session and zeros the flags and actions
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		P3ServerSession(void);

		/**
		 * Sets an flag
		 *
		 * @Param {int64_t} mask
		 * @Return {void}
		 */
		void setFlag(int64_t mask);

		/**
		 * Gets an flag
		 *
		 * @Param {int64_t} mask
		 * @Return {bool}
		 */
		bool getFlag(int64_t mask);

		/**
		 * Sets an action
		 *
		 * @Param {int64_t} mask
		 * @Return {void}
		 */
		void setAction(int64_t mask);

		/**
		 * Gets an action
		 *
		 * @Param {int64_t} mask
		 * @Return {bool}
		 */
		bool getAction(int64_t mask);

		std::string s_User;
		std::string s_Pass;
		AccountShortcut s_Account;
		std::vector<std::tuple<CassUuid, int64_t, int64_t>> s_References;
	private:
		int64_t s_Flags;
		int64_t s_Actions;
	};
}
