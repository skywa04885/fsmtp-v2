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

#include "IMAPCommandParser.src.h"

namespace FSMTP::IMAP::CommandParser
{
	// ======================================
	// Token
	// ======================================

	/**
	 * The default empty constructor
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	Token::Token(void)
	{}

	/**
	 * The default empty constructor
	 *
	 * @Param {const char *} t_String
	 * @Param {const TokenType} t_Type
	 * @Return {void}
	 */
	Token::Token(const char *t_String, const TokenType t_Type):
		t_Type(t_Type)
	{
		std::size_t stringLen = strlen(t_String);
		++stringLen;

		this->t_String = new char[stringLen];
		memcpy(this->t_String, t_String, stringLen);
	}

	/**
	 * The default empty constructor
	 *
	 * @Param {const char} t_Char
	 * @Param {const TokenType} t_Type
	 * @Return {void}
	 */
	Token::Token(const char t_Char, const TokenType t_Type):
		t_Type(t_Type), t_Char(t_Char)
	{}
	
	/**
	 * The default empty constructor
	 *
	 * @Param {const int32_t} t_Int32
	 * @Param {const TokenType} t_Type
	 * @Return {void}
	 */
	Token::Token(const int32_t t_Int32, const TokenType t_Type):
		t_Type(t_Type), t_Int32(t_Int32)
	{}

	/**
	 * The default empty constructor
	 *
	 * @Param {const TokenType} t_Type
	 * @Return {void}
	 */
	Token::Token(const TokenType t_Type):
		t_Type(t_Type)
	{}

	/**
	 * Frees the memory, if string
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void Token::free(void)
	{
		if (this->t_Type == TokenType::TT_OTHER)
			delete[] this->t_String;
	}

	/**
	 * Gets the name of an token type
	 *
	 * @Param {const TokenType} type
	 * @Return {const char *}
	 */
	const char *tokenTypeString(const TokenType type)
	{
		switch (type)
		{
			case TT_LPAREN: return "LPAREN";
			case TT_RPAREN: return "RPAREN";
			case TT_QUOTE: return "QUOTE";
			case TT_OTHER: return "OTHER";
			case TT_SPACE: return "SPACE";
			case TT_LBRACKET: return "LBRACKET";
			case TT_RBRACKET: return "RBRACKET";
			case TT_NUMBER: return "NUMBER";
			case TT_WSP: return "WSP";
			default: throw std::runtime_error("Not implemented");
		}
	}

	/**
	 * Turns the token into an string, for debug
	 *
	 * @param {void}
	 * @Return {std::string}
	 */
	std::string Token::toString(void) const
	{
		std::string ret = tokenTypeString(this->t_Type);
		ret += ": ";

		switch (this->t_Type)
		{
			case TokenType::TT_LPAREN:
			{
				ret += '(';
				break;
			}
			case TokenType::TT_RPAREN:
			{
				ret += ')';
				break;
			}
			case TokenType::TT_QUOTE:
			{
				ret += '"';
				break;
			}
			case TokenType::TT_OTHER:
			{
				ret += this->t_String;
				break;
			}
			case TokenType::TT_SPACE:
			{
				ret += ' ';
				break;
			}
			case TokenType::TT_LBRACKET:
			{
				ret += '[';
				break;
			}
			case TokenType::TT_RBRACKET:
			{
				ret += ']';
				break;
			}
			case TokenType::TT_NUMBER:
			{
				ret += std::to_string(this->t_Int32);
				break;
			}
			case TokenType::TT_WSP:
			{
				ret += ' ';
				break;
			}
			case TokenType::TT_COLON:
			{
				ret += ':';
				break;
			}
			default: throw std::runtime_error("Not implemented");
		}

		return ret;
	}

	// ======================================
	// Lexer
	// ======================================

	static std::string numbers = "0123456789";

	void Lexer::advance(void)
	{
		++this->l_Pos;
		if (this->l_Pos < this->l_Text.length())
			this->l_CurrentChar = this->l_Text[this->l_Pos];
		else
			this->l_CurrentChar = '\0';
	}

	Lexer::Lexer(const std::string &l_Text):
		l_Text(l_Text), l_Pos(-1)
	{
		this->advance();
	}

	/**
	 * Starts making the tokens
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void Lexer::makeTokens(void)
	{
		while (this->l_CurrentChar != '\0')
		{
			if (this->l_CurrentChar == ')')
			{ // RPAREN
				this->l_Tokens.emplace_back(')', TT_LPAREN);
				this->advance();
			} else if (this->l_CurrentChar == '(')
			{ // LPAREN
				this->l_Tokens.emplace_back('(', TT_RPAREN);
				this->advance();
			} else if (this->l_CurrentChar == '"')
			{ // -> QUOTE
				this->l_Tokens.emplace_back('"', TT_QUOTE);
				this->advance();
			} else if (this->l_CurrentChar == '[')
			{ // -> RBRACKET
				this->l_Tokens.emplace_back('[', TT_RBRACKET);
				this->advance();
			} else if (this->l_CurrentChar == ']')
			{ // -> LBRACKET
				this->l_Tokens.emplace_back(']', TT_LBRACKET);
				this->advance();
			} else if (this->l_CurrentChar == ' ')
			{ // -> WSP
				this->l_Tokens.emplace_back(' ', TT_WSP);
				this->advance();
			} else if (this->l_CurrentChar == ':')
			{ // -> WSP
				this->l_Tokens.emplace_back(':', TT_COLON);
				this->advance();
			} else if (numbers.find(this->l_CurrentChar) != std::string::npos)
			{ // - Process the number
				this->makeNumber();
				continue; // To skip the advance
			} else if (
				this->l_CurrentChar != ' ' && this->l_CurrentChar != '[' && 
				this->l_CurrentChar != ')' && this->l_CurrentChar != '(' &&
				this->l_CurrentChar != '[' && this->l_CurrentChar != ']' &&
				this->l_CurrentChar != '"'
			)
			{ // -> Processes an string
				this->makeOther();
				continue;
			} else throw SyntaxException(this->l_Pos, "Invalid char");
		}
	}

	/**
	 * Makes an string of characters
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void Lexer::makeOther(void)
	{
		std::string otherBuf;

		// Gets all the numbers in sequence
		while (
			this->l_CurrentChar != ' ' && this->l_CurrentChar != '[' && 
			this->l_CurrentChar != ')' && this->l_CurrentChar != '(' &&
			this->l_CurrentChar != '[' && this->l_CurrentChar != ']' &&
			this->l_CurrentChar != '"'
		)
		{
			if (this->l_CurrentChar == '\0') break;
			otherBuf += this->l_CurrentChar;
			this->advance();
		}

		// Puts the number in the tokens
		this->l_Tokens.emplace_back(
			otherBuf.c_str(),
			TT_OTHER
		);
	}

	/**
	 * Makes an number of a set of tokens
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void Lexer::makeNumber(void)
	{
		std::string numBuf;

		// Gets all the numbers in sequence
		while (numbers.find(this->l_CurrentChar) != std::string::npos)
		{
			if (this->l_CurrentChar == '\0') break;
			numBuf += this->l_CurrentChar;
			this->advance();
		}

		// Puts the number in the tokens
		int32_t num = std::stoi(numBuf);
		this->l_Tokens.emplace_back(
			num,
			TT_NUMBER
		);
	}

	/**
	 * Default destructor, which free's the memory
	 * 
	 * @Param {void}
	 * @Return {void}
	 */
	Lexer::~Lexer(void)
	{
		for_each(this->l_Tokens.begin(), this->l_Tokens.end(), [=](Token &token){
			token.free();
		});
	}

	// ======================================
	// Parser
	// ======================================

	/**
	 * Default constructor for the parser
	 *
	 * @Param {const std::vector<Token> &} p_Tokens
	 * @Return {void}
	 */
	Parser::Parser(const std::vector<Token> &p_Tokens):
		p_Tokens(p_Tokens), p_TokenIndex(-1), p_CurrentToken(nullptr)
	{}

	/**
	 * Starts the parsing, and pushes the result to
	 * - the target
	 *
	 * @Param {std::vector<Argument> &} target
	 * @Return {void}
	 */
	void Parser::parse(std::vector<Argument> &target)
	{
		this->advance();
		this->analyze(target, true);
	}

	/**
	 * Analyzes and selects which action to start based
	 * - on the then current token
	 *
	 * @Param {std::vector<std::unique_ptr<Node>> &} target
	 * @Param {const bool} head
	 * @Return {void}
	 */
	void Parser::analyze(std::vector<Argument> &target, const bool head)
	{
		switch (this->p_CurrentToken->t_Type)
		{
			case TT_RPAREN:
			{// (asd)
				this->list(target);
				break;
			}
			case TT_RBRACKET:
			{ // [asd]
				this->section(target);
				break;
			}
			case TT_QUOTE:
			{// "asd"
				this->string(target);
				break;
			}
			case TT_NUMBER:
			{// 1234
				this->number(target);
				break;
			}
			case TT_WSP: break;
			case TT_OTHER:
			{ // \ABC
				this->other(target);
				break;
			}
			case TT_COLON:
			{ // 1:4
				this->range(target);
				break;
			}
			default:
			{
				throw std::runtime_error("Not implemented");
			}
		}

		// Checks if we're in the head, and need to scan
		// - for another argument
		if (head)
		{
			this->advance();
			if (this->p_CurrentToken != nullptr)
			{
				this->analyze(target, true);
			}
		}
	}

	/**
	 * Goes to the next token, and sets the pointer to nullptr
	 * - if EOF
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void Parser::advance(void)
	{
		++this->p_TokenIndex;
		if (this->p_TokenIndex < this->p_Tokens.size())
			this->p_CurrentToken = &this->p_Tokens[p_TokenIndex];
		else
			this->p_CurrentToken = nullptr;
	}

	/**
	 * Builds an section node in the recursive manner
	 *
	 * @Param {std::vector<std::unique_ptr<Node>> &} target
	 * @Return {bool}
	 */
	bool Parser::range(std::vector<Argument> &target)
	{
		// Checks if there are enough numbers for an range
		if (target.size() < 1)
			throw SyntaxException(this->p_TokenIndex, "Invalid range !");

		// Checks if the next and previous token is an number
		if (
			this->p_Tokens[this->p_TokenIndex - 1].t_Type != TT_NUMBER ||
			(this->p_Tokens[this->p_TokenIndex + 1].t_Type != TT_NUMBER &&
				this->p_Tokens[this->p_TokenIndex + 1].t_Type != TT_OTHER)
		)
		{
			throw SyntaxException(this->p_TokenIndex, "Range requires two numbers");
		}

		// Removes the previous number, since this is 
		// - now part of the range
		target.pop_back();

		// Checks what kind of node we create
		int32_t from;
		if (this->p_Tokens[this->p_TokenIndex + 1].t_Type == TT_OTHER)
			from = -1;
		else
			from = this->p_Tokens[this->p_TokenIndex - 1].t_Int32;

		// Checks what kind of to to create
		int32_t to;
		if (this->p_Tokens[this->p_TokenIndex + 1].t_Type == TT_OTHER)
			to = -1;
		else
			to = this->p_Tokens[this->p_TokenIndex + 1].t_Int32;

		// Creates the node
		target.push_back(Argument(Range {
			from, 
			to
		}, ArgumentType::A_TYPE_RANGE));

		// Goes to the next token and returns
		this->advance();
		return true;
	}

	/**
	 * Parses an number and pushes it to the target
	 *
	 * @Param {std::vector<std::unique_ptr<Node>> &} target
	 * @Return {bool}
	 */
	bool Parser::number(std::vector<Argument> &target)
	{
		const Token &token = *this->p_CurrentToken;

		if (token.t_Type == TT_NUMBER)
		{
			target.push_back(Argument(token.t_Int32, ArgumentType::A_TYPE_INT32));
			return true;
		} else return false;
	}

	/**
	 * Parses an atom and pushes it to the target
	 *
	 * @Param {std::vector<std::unique_ptr<Node>> &} target
	 * @Return {bool}
	 */
	bool Parser::other(std::vector<Argument> &target)
	{
		const Token &token = *this->p_CurrentToken;

		target.push_back(Argument(token.t_String, ArgumentType::A_TYPE_ATOM));
		return true;
	}

	/**
	 * Parses an string and pushes it to the target
	 *
	 * @Param {std::vector<std::unique_ptr<Node>> &} target
	 * @Return {bool}
	 */
	bool Parser::string(std::vector<Argument> &target)
	{
		std::string content;

		while (true)
		{
			this->advance();

			// Checks if we've reached the end
			if (this->p_CurrentToken == nullptr)
				throw SyntaxException(this->p_TokenIndex, "Closing quote not found !");

			// Checks if it is an closing quote, if so
			// - break and push the result
			if (this->p_CurrentToken->t_Type == TT_QUOTE)
			{
				target.push_back(Argument(content.c_str(), ArgumentType::A_TYPE_STRING));
				return true;
			}

			// Gets the token reference, and checks the type
			// - and appends it to the content
			const Token &token = *this->p_CurrentToken;
			if (token.t_Type == TT_OTHER) content += token.t_String;
			else if (token.t_Type == TT_NUMBER) content += std::to_string(token.t_Int32);
			else content += token.t_Char;
		}

		return false;
	}

	/**
	 * Builds an list node in the recursive manner
	 *
	 * @Param {std::vector<std::unique_ptr<Node>> &target}
	 * @Return {bool}
	 */
	bool Parser::list(std::vector<Argument> &target)
	{
		target.push_back(Argument(0, ArgumentType::A_TYPE_LIST));
		Argument &arg = target[target.size() - 1];

		while (true)
		{
			this->advance();

			// Checks if we've reached the end
			if (this->p_CurrentToken == nullptr)
				throw SyntaxException(this->p_TokenIndex, "Closing paren not found");

			// Checks if we've reached the end, then we push
			// - the result
			if (this->p_CurrentToken->t_Type == TT_LPAREN)
				return true;

			// Performs the action
			if (this->p_CurrentToken->t_Type == TT_WSP) continue;
			this->analyze(arg.a_Children, false);
		}

		return false;
	};

	/**
	 * Builds an section node in the recursive manner
	 *
	 * @Param {std::vector<std::unique_ptr<Node>> &} target
	 * @Return {bool}
	 */
	bool Parser::section(std::vector<Argument> &target)
	{
		target.push_back(Argument(0, ArgumentType::A_TYPE_SECTION));
		Argument &arg = target[target.size() - 1];

		while (true)
		{
			this->advance();

			// Checks if we've reached the end
			if (this->p_CurrentToken == nullptr)
				throw SyntaxException(this->p_TokenIndex, "Closing paren not found");

			// Checks if we've reached the end, then we push
			// - the result
			if (this->p_CurrentToken->t_Type == TT_LBRACKET)
				return true;

			// Performs the action
			if (this->p_CurrentToken->t_Type == TT_WSP) continue;
			this->analyze(arg.a_Children, false);
		}

		return false;
	}

	/**
	 * Default empty constructor
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	Argument::Argument(void)
	{}

	/**
	 * Integer argument
	 *
	 * @Param {const int32_t} a_Int32
	 * @Param {const ArgumentType} a_Type
	 * @Return {void}
	 */
	Argument::Argument(const int32_t a_Int32, const ArgumentType a_Type):
		a_Int32(a_Int32), a_Type(a_Type)
	{}

	/**
	 * Integer argument
	 *
	 * @Param {const char} a_Char
	 * @Param {const ArgumentType} 
	 * @Return {void}
	 */
	Argument::Argument(const char a_Char, const ArgumentType a_Type):
		a_Char(a_Char), a_Type(a_Type)
	{}

	/**
	 * Integer argument
	 *
	 * @Param {const Range} a_Range
	 * @Param {const ArgumentType} 
	 * @Return {void}
	 */
	Argument::Argument(const Range a_Range, const ArgumentType a_Type):
		a_Range(a_Range), a_Type(a_Type)
	{}

	/**
	 * Integer argument
	 *
	 * @Param {const char *} a_String
	 * @Param {const ArgumentType} 
	 * @Return {void}
	 */
	Argument::Argument(const char *a_String, const ArgumentType a_Type):
		a_Type(a_Type)
	{
		std::size_t stringLen = strlen(a_String);
		++stringLen;

		this->a_String = new char[stringLen];
		memcpy(this->a_String, a_String, stringLen);
	}

	/**
	 * Deletes the string if it is there
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void Argument::free(void)
	{
		if (this->a_Type == A_TYPE_STRING || this->a_Type == A_TYPE_ATOM)
			delete[] this->a_String;

		for_each(this->a_Children.begin(), this->a_Children.end(), [=](Argument &a)
		{
			a.free();
		});
	}
}