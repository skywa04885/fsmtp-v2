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

		std::string toString(void) const;

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

		/**
		 * Starts making the tokens
		 *
		 * @Param {void}
		 * @Return {void}
		 */
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

	class Node
	{
	public:
		virtual std::string toString(void) = 0;

		std::vector<std::unique_ptr<Node>> n_Nodes;
	};

	class StringNode : public Node
	{ // "DATA"
	public:
		StringNode(const std::string &n_Value);

		virtual std::string toString(void);

		std::string n_Value;
	};

	class NumberNode : public Node
	{ // 1234
	public:
		NumberNode(const int32_t n_Value);

		virtual std::string toString(void);

		int32_t n_Value;
	};

	class SectionNode : public Node
	{ // [DATA]
	public:
		SectionNode(void);

		virtual std::string toString(void);
	};

	class ListNode : public Node
	{ // (DATA, DATA1, DATA2)
	public:
		ListNode(void);

		virtual std::string toString(void);
	};

	class AtomNode : public Node
	{ // "DATA"
	public:
		AtomNode(const std::string &n_Value);

		virtual std::string toString(void);

		std::string n_Value;
	};

	class Parser
	{
	public:
		Parser(const std::vector<Token> &p_Tokens);

		void parse(void);

		/**
		 * Analyzes and selects which action to start based
		 * - on the then current token
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
	 	 * @Param {const bool} head
		 * @Return {void}
		 */
		void analyze(std::vector<std::unique_ptr<Node>> &target, const bool head);

		/**
		 * Goes to the next token, and sets the pointer to nullptr
		 * - if EOF
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void advance(void);

		/**
		 * Parses an number and pushes it to the target
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
		 * @Return {bool}
		 */
		bool number(std::vector<std::unique_ptr<Node>> &target);

		/**
		 * Parses an string and pushes it to the target
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
		 * @Return {bool}
		 */
		bool string(std::vector<std::unique_ptr<Node>> &target);

		/**
		 * Parses an atom and pushes it to the target
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
		 * @Return {bool}
		 */
		bool other(std::vector<std::unique_ptr<Node>> &target);

		/**
		 * Builds an list node in the recursive manner
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
		 * @Return {bool}
		 */
		bool list(std::vector<std::unique_ptr<Node>> &target);

		std::vector<std::unique_ptr<Node>> p_Nodes;
	private:
		const Token *p_CurrentToken;
		char p_CurrentChar;
		std::size_t p_TokenIndex;
		const std::vector<Token> &p_Tokens;
	};
}