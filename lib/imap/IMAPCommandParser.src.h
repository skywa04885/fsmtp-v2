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
		TT_LPAREN = 0,
		TT_RPAREN,
		TT_QUOTE,
		TT_OTHER,
		TT_SPACE,
		TT_LBRACKET,
		TT_RBRACKET,
		TT_NUMBER,
		TT_WSP,
		TT_COLON
	} TokenType;

	class SyntaxException
	{
	public:
		SyntaxException(const std::size_t i, const std::string &e_Message):
			e_Message("ERR [token: " + std::to_string(i) + "]: " + e_Message)
		{}

		const char *what(void) const throw()
		{
			return this->e_Message.c_str();
		}
	private:
		std::string e_Message;
	};

	/**
	 * Gets the name of an token type
	 *
	 * @Param {const TokenType} type
	 * @Return {const char *}
	 */
	const char *tokenTypeString(const TokenType type);

	class Token
	{
	public:
		/**
		 * The default empty constructor
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit Token(void);

		/**
		 * The default empty constructor
		 *
		 * @Param {const char *} t_String
		 * @Param {const TokenType} t_Type
		 * @Return {void}
		 */
		Token(const char *t_String, const TokenType t_Type);

		/**
		 * The default empty constructor
		 *
		 * @Param {const char} t_Char
		 * @Param {const TokenType} t_Type
		 * @Return {void}
		 */
		Token(const char t_Char, const TokenType t_Type);

		/**
		 * The default empty constructor
		 *
		 * @Param {const int32_t} t_Int32
		 * @Param {const TokenType} t_Type
		 * @Return {void}
		 */
		Token(const int32_t t_Int32, const TokenType t_Type);

		/**
		 * The default empty constructor
		 *
		 * @Param {const TokenType} t_Type
		 * @Return {void}
		 */
		Token(const TokenType t_Type);

		/**
		 * Frees the memory, if string
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void free(void);

		/**
		 * Turns the token into an string, for debug
		 *
		 * @param {void}
		 * @Return {std::string}
		 */
		std::string toString(void) const;

		union
		{
			char *t_String;
			int32_t t_Int32;
			char t_Char;
		};

		TokenType t_Type;
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

		/**
		 * Makes an number of a set of tokens
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void makeNumber(void);

		/**
		 * Makes an string of characters
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void makeOther(void);

		/**
		 * Default destructor, which free's the memory
		 * 
		 * @Param {void}
		 * @Return {void}
		 */
		~Lexer(void);

		std::vector<Token> l_Tokens;
	private:
		std::string l_Text;
		std::size_t l_Pos;
		char l_CurrentChar;
	};

	typedef enum : uint8_t
	{
		A_TYPE_STRING = 0,
		A_TYPE_INT32,
		A_TYPE_INT64,
		A_TYPE_CHAR,
		A_TYPE_ATOM,
		A_TYPE_SECTION,
		A_TYPE_LIST,
		A_TYPE_RANGE
	} ArgumentType;

	typedef struct
	{
		int32_t r_From;
		int32_t r_To;
	} Range;

	class Argument
	{
	public:
		/**
		 * Default empty constructor
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit Argument(void);

		/**
		 * Integer argument
		 *
		 * @Param {const int32_t} a_Int32
		 * @Param {const ArgumentType} a_Type
		 * @Return {void}
		 */
		Argument(const int32_t a_Int32, const ArgumentType a_Type);

		/**
		 * Integer argument
		 *
		 * @Param {const int32_t} a_Int32
		 * @Param {const ArgumentType} 
		 * @Return {void}
		 */
		Argument(const char a_Char, const ArgumentType a_Type);

		/**
		 * Integer argument
		 *
		 * @Param {const Range} a_Range
		 * @Param {const ArgumentType} 
		 * @Return {void}
		 */
		Argument(const Range a_Range, const ArgumentType a_Type);

		/**
		 * Integer argument
		 *
		 * @Param {const char *} a_String
		 * @Param {const ArgumentType} 
		 * @Return {void}
		 */
		Argument(const char *a_String, const ArgumentType a_Type);

		/**
		 * Deletes the string if it is there
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void free(void);

		union
		{
			int32_t a_Int32;
			char *a_String;
			char a_Char;
			Range a_Range;
		};

		std::vector<Argument> a_Children;
		ArgumentType a_Type;
	};

	class Parser
	{
	public:
		/**
		 * Default constructor for the parser
		 *
		 * @Param {const std::vector<Token> &} p_Tokens
		 * @Return {void}
		 */
		Parser(const std::vector<Token> &p_Tokens);

		/**
		 * Starts the parsing, and pushes the result to
		 * - the target
		 *
		 * @Param {std::vector<Argument> &} target
		 * @Return {void}
		 */
		void parse(std::vector<Argument> &target);

		/**
		 * Analyzes and selects which action to start based
		 * - on the then current token
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
	 	 * @Param {const bool} head
		 * @Return {void}
		 */
		void analyze(std::vector<Argument> &target, const bool head);

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
		bool number(std::vector<Argument> &target);

		/**
		 * Parses an string and pushes it to the target
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
		 * @Return {bool}
		 */
		bool string(std::vector<Argument> &target);

		/**
		 * Parses an atom and pushes it to the target
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
		 * @Return {bool}
		 */
		bool other(std::vector<Argument> &target);

		/**
		 * Builds an list node in the recursive manner
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
		 * @Return {bool}
		 */
		bool list(std::vector<Argument> &target);

		/**
		 * Builds an section node in the recursive manner
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
		 * @Return {bool}
		 */
		bool section(std::vector<Argument> &target);

		/**
		 * Builds an section node in the recursive manner
		 *
		 * @Param {std::vector<std::unique_ptr<Node>> &} target
		 * @Return {bool}
		 */
		bool range(std::vector<Argument> &target);
	private:
		const Token *p_CurrentToken;
		char p_CurrentChar;
		std::size_t p_TokenIndex;
		const std::vector<Token> &p_Tokens;
	};
}