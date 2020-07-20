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

#include "IMAP.src.h"

namespace FSMTP::IMAP
{
	typedef enum : uint32_t
	{
		IRT_GREETING = 0	
	} IMAPResponseType;

	class IMAPResponse
	{
	public:
		/**
		 * Default constructor for the imap response
		 *
		 * @Param {const IMAPResponseType} r_Type
		 * @Return {void}
		 */
		explicit IMAPResponse(const IMAPResponseType r_Type);

		/**
		 * Builds the response
		 *
		 * @Param {const IMAPResponseType} r_Type
		 * @Return {void}
		 */
		std::string build(void);

		/**
		 * Gets the message
		 *
		 * @Param {void}
		 * @Return {std::string}
		 */
		std::string getMessage(void);
	private:
		std::string r_Message;
		IMAPResponseType r_Type;
		void *r_U;
	};
}