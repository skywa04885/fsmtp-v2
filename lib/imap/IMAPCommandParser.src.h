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

namespace FSMTP::IMAP::CommandParser
{
	typedef enum : uint8_t
	{
		TVT_STRING = 0,
		TVT_CHAR,
		TVT_INT32,
		TVT_INT64
	} TokenValueType;

	typedef enum : uint8_t
	{
		TT_LPAREN = 0,
		TT_RPAREN,
		TT_QUOTE,
		TT_OTHER,
		TT_COMMA,
		TT_SPACE,
		TT_LBRACKET,
		TT_RBRACKET,
		TT_NUMBER,
		TT_WSP
	} TokenType;

	const char *tokenTypeString(const TokenType type);

	class Token
	{
	public:
		Token(const TokenType t_Type, const TokenValueType t_ValueType, const void *value);
		void freeMem(void);

		const char *getString(void) const;
		char getChar(void) const;
		int32_t getInt32(void) const;
		int64_t getInt64(void) const;

		std::string stringRepresentation(void) const;

		TokenType t_Type;
	private:
		void *t_Value;
		TokenValueType t_ValueType;
	};

	class Lexer
	{
	public:
		Lexer(const std::string &l_Text);

		void advance(void);

		void makeTokens(void);

		void makeNumber(void);

		void makeOther(void);

		~Lexer(void);

		std::vector<Token> l_Tokens;
	private:
		std::string l_Text;
		std::size_t l_Pos;
		char l_CurrentChar;
	};

	class Parser
	{
	public:
	private:
	};
}