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

#include "IMAPCommand.src.h"
#include "IMAP.src.h"

namespace FSMTP::IMAP
{
	/**
	 * Default emty constructor
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	IMAPCommand::IMAPCommand(void):
		c_Type(ICT_UNKNOWN)
	{}

	/**
	 * Parses an IMAP command
	 *
	 * @Param {const std::string &} raw
	 * @Return {void}
	 */
	IMAPCommand::IMAPCommand(const std::string &raw):
		c_Type(ICT_UNKNOWN)
	{
		this->parse(raw);
	}

	/**
	 * Parses an IMAP command
	 *
	 * @Param {const std::string &} raw
	 * @Return {void}
	 */
	void IMAPCommand::parse(const std::string &raw)
	{
		// =======================================
		// Cleans the message
		//
		// Makes it easier for us to process
		// =======================================

		std::string clean;

		// Reduces whitespace to one occurance,
		// - and removes the first and last
		reduceWhitespace(raw, clean);
		removeFirstAndLastWhite(clean);

		// =======================================
		// Proceses the command
		//
		// Splits the command into sections
		// =======================================

		// Checks if it is allowed to run the specified command
		// - without specifying the index
		std::size_t firstSpace = clean.find_first_of(' ');
		if (firstSpace == std::string::npos)
		{
			if (clean.empty()) this->c_Index = "*";
			else this->c_Index = clean;
			throw IMAPBad("[CLIENTBUG] Command has no tag set");	
		} else
		{
			// Splits the index from the rest of the command
			this->c_Index = clean.substr(0, firstSpace);
		}

		// Gets the other parts of the command, if there is nothing
		// - throw bad since the command is invalid
		std::string other = clean.substr(firstSpace);
		if (other[0] == ' ') other.erase(0, 1);

		// Parses the command from the other data
		// - if there is no command, throw error
		std::size_t commandEndIndex = other.find_first_of(' ');
		if (commandEndIndex == std::string::npos)
			commandEndIndex = other.size();

		std::string command = other.substr(0, commandEndIndex);
		this->getType(command);
		if (this->c_Type == IMAPCommandType::ICT_UNKNOWN)
			throw IMAPBad("[CLIENTBUG] Command is not an valid IMAP command");

		// Parses the command arguments, and if empty
		// - just ignore
		std::string arguments = other.substr(commandEndIndex);
		if (arguments[0] == ' ') arguments.erase(0, 1);
		if (!arguments.empty())
		{
			this->parseArguments(arguments);
		}
	}


	/**
	 * Parses the arguments
	 *
	 * @Param {const std::string &} raw
	 * @Return {void}
	 */
	void IMAPCommand::parseArguments(const std::string &raw)
	{
		try
		{
			// Performs the lexical analysis
			CommandParser::Lexer lexer(raw);
			lexer.makeTokens();

			// Parses the tokens
			CommandParser::Parser parser(lexer.l_Tokens);
			parser.parse(this->c_Args);
		} catch (const SyntaxException &e)
		{
			std::string error = "Syntax error: ";
			error += e.what();
			throw IMAPBad(EXCEPT_DEBUG(error));
		}
	}

	/**
	 * Gets and sets the type of the command
	 *
	 * @Param {std::string} command
	 * @Return {void}
	 */
	void IMAPCommand::getType(std::string command)
	{
		/*
			-> Will get pretty messy here
		*/

		// Makes the command lowercase
		std::transform(command.begin(), command.end(), command.begin(), [](char c) {
			return std::tolower(static_cast<int>(c));
		});

		// Checks the command type and stores it inside
		// - of the current class
		switch (command[0])
		{
			case 'n':
			{ // NOOP
				if (command == "noop") this->c_Type = IMAPCommandType::ICT_NOOP;
				else this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 's':
			{ // STARTTLS, SELECT, STATUS, STORE, SUBSCRIBE
				if (command == "starttls") this->c_Type = IMAPCommandType::ICT_STARTTLS;
				else if (command == "select") this->c_Type = IMAPCommandType::ICT_SELECT;
				else if (command == "status") this->c_Type = IMAPCommandType::ICT_STATUS;
				else if (command == "store") this->c_Type = IMAPCommandType::ICT_STORE;
				else if (command == "subscribe") this->c_Type = IMAPCommandType::ICT_SUBSCRIBE;
				else this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 'a':
			{ // AUTHENTICATE, APPEND
				if (command == "authenticate") this->c_Type = IMAPCommandType::ICT_AUTHENTICATE;
				else if (command == "append") this->c_Type = IMAPCommandType::ICT_APPEND;
				else this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 'l':
			{ // LOGIN, LIST, LSUB, LOGOUT
				if (command == "login") this->c_Type = IMAPCommandType::ICT_LOGIN;
				else if (command == "list") this->c_Type = IMAPCommandType::ICT_LIST;
				else if (command == "lsub") this->c_Type = IMAPCommandType::ICT_LSUB;
				else if (command == "logout") this->c_Type = IMAPCommandType::ICT_LOGOUT;
				else this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 'e':
			{ // EXAMINE
				if (command == "examine") this->c_Type = IMAPCommandType::ICT_EXAMINE;
				else this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 'c':
			{ // CREATE, CHECK, CLOSE, COPY, CAPABILITY
				if (command == "create") this->c_Type = IMAPCommandType::ICT_CREATE;
				else if (command == "check") this->c_Type = IMAPCommandType::ICT_CHECK;
				else if (command == "close") this->c_Type = IMAPCommandType::ICT_CLOSE;
				else if (command == "copy") this->c_Type = IMAPCommandType::ICT_COPY;
				else if (command == "capability") this->c_Type = IMAPCommandType::ICT_CAPABILITY;
				else this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 'd':
			{ // DELETE
				if (command == "delete") this->c_Type = IMAPCommandType::ICT_DELETE;
				else this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 'u':
			{ // UNSUBSCRIBE, UID
				if (command == "unsubscribe") this->c_Type = IMAPCommandType::ICT_UNSUBSCRIBE;
				else if (command == "uid") this->c_Type = IMAPCommandType::ICT_UID;
				else this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 'f':
			{ // FETCH
				if (command == "fetch") this->c_Type = IMAPCommandType::ICT_FETCH;
				else this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			default: this->c_Type = IMAPCommandType::ICT_UNKNOWN;
		}
	}

	/**
	 * Free's the memory
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	IMAPCommand::~IMAPCommand(void)
	{
		for_each(this->c_Args.begin(), this->c_Args.end(), [=](Argument &a)
		{
			a.free();
		});
	}
}
