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
		PRT_GREETING = 0,
		PRT_CAPA,
		PRT_STLS_START,
		PRT_COMMAND_INVALID,
		PRT_USER_DONE,
		PRT_AUTH_SUCCESS,
		PRT_AUTH_FAIL,
		PRT_SYNTAX_ERROR,
		PRT_QUIT,
		PRT_STAT,
		PRT_UIDL,
		PRT_LIST,
		PRT_RETR,
		PRT_DELE_SUCCESS,
		PRT_ORDER_ERROR,
		PRT_TOP,
		PRT_RSET,
		PRT_IMPLEMENTATION
	} POP3ResponseType;

	typedef struct
	{
		const char *c_Name;
		std::vector<const char *> c_Args;
	} POP3Capability;

	typedef struct
	{
		std::size_t e_Index;
		std::string e_Value;
	} POP3ListElement;

	class P3Response
	{
	public:
		P3Response(const bool p_Ok, const POP3ResponseType p_Type);

		P3Response(
			const bool p_Ok,
			const POP3ResponseType p_Type,
			const std::string &p_Message,
			std::vector<POP3Capability> *p_Capabilities,
			std::vector<POP3ListElement> *p_ListElements,
			const void *p_U
		);

		/**
		 * Builds the list of capabilities
		 *
		 * @Param {void}
		 * @Return {std::string}
		 */
		std::string buildCapabilities(void);

		/**
		 * Builds an list of index and value pairs
		 *
		 * @Param {void}
		 * @Return {std::string}
		 */
		std::string buildList(void);

		std::string build(void);

		std::string getMessage(void);
	private:
		bool p_Ok;
		POP3ResponseType p_Type;
		std::string p_Message;
		std::vector<POP3Capability> *p_Capabilities;
		std::vector<POP3ListElement> *p_ListElements;
		const void *p_U;
	};
}
