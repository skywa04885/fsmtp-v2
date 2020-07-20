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
		IRT_GREETING = 0,
		IRT_CAPABILITIES,
		IRT_LOGOUT,
		IRT_ERR
	} IMAPResponseType;

	typedef enum : uint32_t
	{
		IPT_BAD = 0,
		IPT_OK,
		IPT_NO,
		IPT_BYE
	} IMAPResponsePrefixType;

	typedef enum : uint8_t
	{
		IRS_NTATL = 0, 	// Non tagged line, followed by tagged line
		IRS_NT, 				// Non tagged line only
		IRS_TL 					// Tagged line only
	} IMAPResponseStructure;

	typedef struct
	{
		const char *c_Key;
		const char *c_Value;
	} IMAPCapability;

	class IMAPResponse
	{
	public:
		/**
		 * Default constructor for the imap response
		 *
		 * @Param {const IMAPResponseType} r_Type
		 * @Param {const std::string} r_TagIndex
		 * @Param {const IMAPResponseStructure} r_structure
		 * @Param {const IMAPResponsePrefixType} r_PrefType
		 * @Param {void *} r_U
		 * @Return {void}
		 */
		explicit IMAPResponse(
			const IMAPResponseStructure r_Structure,
			const std::string r_TagIndex,
			const IMAPResponseType r_Type,
			const IMAPResponsePrefixType r_PrefType,
			void *r_U
		);

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

		/**
		 * Gets the name of the command type
		 *
		 * @Param {void}
		 * @Return {const char *}
		 */
		const char *getCommandName(void);

		/**
		 * Gets an command prefix
		 *
		 * @Param {void}
		 * @Return {const char *} 
		 */
		const char *getPrefix(void);

		static std::string buildCapabilities(
			const std::vector<IMAPCapability> &capabilities
		);
	private:
		std::string r_Message;
		IMAPResponseType r_Type;
		void *r_U;
		IMAPResponseStructure r_Structure;
		std::string r_TagIndex;
		IMAPResponsePrefixType r_PrefType;
	};
}