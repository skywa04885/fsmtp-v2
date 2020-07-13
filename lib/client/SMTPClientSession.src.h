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

#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

#include "../general/macros.src.h"

#define _SCS_ACTION_HELO 1
#define _SCS_ACTION_START_TLS 2
#define _SCS_ACTION_MAIL_FROM 4
#define _SCS_ACTION_RCPT_TO 8
#define _SCS_ACTION_DATA_START 16
#define _SCS_ACTION_DATA_END 32
#define _SCS_ACTION_QUIT 64
#define _SCS_ACTION_START_TLS_FINISHED 128

#define _SCS_FLAG_ESMTP 1
#define _SCS_FLAG_STARTTLS 2
#define _SCS_FLAG_CHUNKING 4
#define _SCS_FLAG_PIPELINGING 8
#define _SCS_FLAG_8BITMIME 16
#define _SCS_FLAG_ENCHANCHED_STATUS_CODES 32

namespace FSMTP::Mailer::Client
{
	class SMTPClientSession
	{
	public:
		/**
		 * Default constructor, sets the flags blank
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit SMTPClientSession();

		/**
		 * Sets an flag
		 *
		 * @Param {int64_t} mask
		 * @Return {void}
		 */
		void setFlag(int64_t mask);
		
		/**
		 * Gets an flag as bool
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
		 * Gets an action as bool
		 *
		 * @Param {int64_t} mask
		 * @Return {bool}
		 */
		bool getAction(int64_t mask);

		/**
		 * Clears an action
		 *
		 * @Param {int64_t} mask
		 * @Return {void}
		 */
		void clearAction(int64_t mask);

		/**
		 * Gets an action and returns the
		 * - bool value, and sets it
		 *
		 * @Param {int64_t mask}
		 * @Return {void}
		 */
		bool getActionSet(int64_t mask);
	private:
		int64_t s_PerformedActions;
		int64_t s_SessionFlags;
	};
}