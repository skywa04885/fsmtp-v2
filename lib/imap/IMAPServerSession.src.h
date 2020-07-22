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

#define _IMAP_FLAG_LOGGED_IN 1

#define _IMAP_ACT_LOGIN 1
#define _IMAP_ACT_AUTH 2

#include "IMAP.src.h"

namespace FSMTP::IMAP
{
	typedef enum : uint8_t
	{
		SST_NO_AUTH = 0,
		SST_AUTH,
		SST_SEL
	} ServerSessionState;

	class IMAPServerSession
	{
	public:
		/**
		 * Zeros the flags
		 * 
		 * @Param {void}
		 * @Return {void}
		 */
		explicit IMAPServerSession(void);

		/**
		 * Sets an flag
		 *
		 * @Param {const int64_t} mask
		 * @Return {void}
		 */
		void setFlag(const int64_t mask);

		/**
		 * Checks if an flag is set or not
		 *
		 * @Param {const int64_t} mask
		 * @Return {bool}
		 */
		bool getFlag(const int64_t mask);

		/**
		 * Sets an action
		 * 
		 * @Param {const int64_t} mask
		 * @Return {void}
		 */
		void setAction(const int64_t mask);

		/**
		 * Checks if an action is set
		 *
		 * @Param {const int64_t} mask
		 * @Param {bool}
		 */
		bool getAction(const int64_t mask);

		/**
		 * Clears an flag
		 *
		 * @Param {const int64_t} mask
		 * @Param {void}
		 */
		void clearFlag(const int64_t mask);

		AccountShortcut s_Account;
		ServerSessionState s_State;
		std::string s_SelectedMailbox;
	private:
		int64_t s_Flags;
		int64_t s_Actions;
	};
}