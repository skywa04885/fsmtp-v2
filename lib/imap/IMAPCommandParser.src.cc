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

	Token::Token(const TokenType t_Type, const TokenValueType t_ValueType, const void *value):
		t_ValueType(t_ValueType), t_Type(t_Type)
	{
		switch (t_ValueType)
		{
			case TVT_STRING:
			{
				std::size_t memLen = strlen(reinterpret_cast<const char *>(value));
				++memLen;
				this->t_Value = malloc(memLen);
				memcpy(this->t_Value, value, memLen);
				break;
			}
			case TVT_CHAR:
			{
				this->t_Value = malloc(sizeof (char));
				memcpy(this->t_Value, value, sizeof (char));
				break;
			}
			case TVT_INT32:
			{
				this->t_Value = malloc(sizeof (int32_t));
				memcpy(this->t_Value, value, sizeof (int32_t));
				break;
			}
			case TVT_INT64:
			{
				this->t_Value = malloc(sizeof(int64_t));
				memcpy(this->t_Value, value, sizeof (int64_t));
				break;
			}
		}
	}

	void Token::freeMem(void)
	{
		free(this->t_Value);
	}

	const char *Token::getString(void) const
	{
		return reinterpret_cast<const char *>(this->t_Value);
	}
	
	char Token::getChar(void) const
	{
		return *reinterpret_cast<char *>(this->t_Value);
	}
	
	int32_t Token::getInt32(void) const
	{
		return *reinterpret_cast<int32_t *>(this->t_Value);
	}

	int64_t Token::getInt64(void) const
	{
		return *reinterpret_cast<int64_t *>(this->t_Value);
	}

	const char *tokenTypeString(const TokenType type)
	{
		switch (type)
		{
			case TT_LPAREN: return "LPAREN";
			case TT_RPAREN: return "RPAREN";
			case TT_QUOTE: return "QUOTE";
			case TT_OTHER: return "OTHER";
			case TT_COMMA: return "COMMA";
			case TT_SPACE: return "SPACE";
			case TT_LBRACKET: return "LBRACKET";
			case TT_RBRACKET: return "RBRACKET";
			case TT_NUMBER: return "NUMBER";
			case TT_WSP: return "WSP";
			default: throw std::runtime_error(EXCEPT_DEBUG("Not implemented"));
		}
	}

	std::string Token::stringRepresentation(void) const
	{
		std::string ret = tokenTypeString(this->t_Type);
		ret += ": ";

		switch (this->t_ValueType)
		{
			case TVT_STRING:
			{
				ret += this->getString();
				break;
			}
			case TVT_CHAR:
			{
				ret += this->getChar();
				break;
			}
			case TVT_INT32:
			{
				ret += std::to_string(this->getInt32());
				break;
			}
			case TVT_INT64:
			{
				ret += std::to_string(this->getInt64());
				break;
			}
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

	void Lexer::makeTokens(void)
	{
		while (this->l_CurrentChar != '\0')
		{
			if (this->l_CurrentChar == ')')
			{ // RPAREN
				this->l_Tokens.emplace_back(
					TT_LPAREN,
					TVT_CHAR, 
					reinterpret_cast<const void *>(")")
				);
				this->advance();
			} else if (this->l_CurrentChar == '(')
			{ // LPAREN
				this->l_Tokens.emplace_back(
					TT_RPAREN,
					TVT_CHAR, 
					reinterpret_cast<const void *>("(")
				);
				this->advance();
			} else if (this->l_CurrentChar == '"')
			{ // -> QUOTE
				this->l_Tokens.emplace_back(
					TT_QUOTE,
					TVT_CHAR, 
					reinterpret_cast<const void *>("\"")
				);
				this->advance();
			} else if (this->l_CurrentChar == ',')
			{ // -> COMMA
				this->l_Tokens.emplace_back(
					TT_COMMA,
					TVT_CHAR, 
					reinterpret_cast<const void *>(",")
				);
				this->advance();
			} else if (this->l_CurrentChar == ']')
			{ // -> RBRACKET
				this->l_Tokens.emplace_back(
					TT_RBRACKET,
					TVT_CHAR, 
					reinterpret_cast<const void *>("]")
				);
				this->advance();
			} else if (this->l_CurrentChar == '[')
			{ // -> LBRACKET
				this->l_Tokens.emplace_back(
					TT_LBRACKET,
					TVT_CHAR, 
					reinterpret_cast<const void *>("[")
				);
				this->advance();
			} else if (this->l_CurrentChar == ' ')
			{ // -> WSP
				this->l_Tokens.emplace_back(
					TT_WSP,
					TVT_CHAR, 
					reinterpret_cast<const void *>(" ")
				);
				this->advance();
			} else if (numbers.find(this->l_CurrentChar) != std::string::npos)
			{ // - Process the number
				this->makeNumber();
				continue; // To skip the advance
			} else if (std::isalpha(this->l_CurrentChar))
			{ // -> Processes an string
				this->makeOther();
				continue;
			} else throw std::runtime_error(EXCEPT_DEBUG("Invalid char"));
		}

		std::cout << "Parsing process step 1 (LEXER): " << std::endl;
		for_each(this->l_Tokens.begin(), this->l_Tokens.end(), [=](const Token &token){
			std::cout << "- " << token.stringRepresentation() << std::endl;
		});
	}

	void Lexer::makeOther(void)
	{
		std::string otherBuf;

		// Gets all the numbers in sequence
		while (std::isalpha(this->l_CurrentChar))
		{
			if (this->l_CurrentChar == '\0') break;
			otherBuf += this->l_CurrentChar;
			this->advance();
		}

		// Puts the number in the tokens
		this->l_Tokens.emplace_back(
			TT_OTHER,
			TVT_STRING, 
			reinterpret_cast<const void *>(otherBuf.c_str())
		);
	}

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
			TT_NUMBER,
			TVT_INT32, 
			reinterpret_cast<const void *>(&num)
		);
	}

	Lexer::~Lexer(void)
	{
		for_each(this->l_Tokens.begin(), this->l_Tokens.end(), [=](Token &token){
			token.freeMem();
		});
	}

	// ======================================
	// Parser
	// ======================================

}