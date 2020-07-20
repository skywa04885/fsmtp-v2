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
	IMAPCommand::IMAPCommand(void)
	{}

	/**
	 * Parses an IMAP command
	 *
	 * @Param {const std::string &} raw
	 * @Return {void}
	 */
	IMAPCommand::IMAPCommand(const std::string &raw)
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
		// Removes the duplicate whitespace
		std::string clean;
		reduceWhitespace(raw, clean);
		removeFirstAndLastWhite(clean);

		// Checks if we should even parse
		if (clean.empty())
		{
			this->c_Type = IMAPCommandType::ICT_UNKNOWN;
			return;
		}

		// Checks where we should perform the separation
		std::size_t sep = clean.find_first_of(' ');
		if (sep == std::string::npos)
		{
			this->c_Index = clean;
			throw SyntaxError("Could not parse command");
		}

		// Parses the index
		this->c_Index = clean.substr(0, sep);

		// Parses the command, and converts it to lower case
		std::string other = clean.substr(sep+1);
		std::size_t cmdArgSep = other.find_first_of(' ');
		if (cmdArgSep == std::string::npos) // -> Command only
			cmdArgSep = other.size();

		std::string command = other.substr(0, cmdArgSep);

		std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c){
			return std::tolower(c);
		});

		// Checks the command type and stores it inside
		// - of the current class
		switch (command[0])
		{
			case 'c':
			{
				if (command == "capability")
					this->c_Type = IMAPCommandType::ICT_CAPABILITY;
				else
					this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 'l':
			{
				if (command == "logout")
					this->c_Type = IMAPCommandType::ICT_LOGOUT;
				else if (command == "login")
					this->c_Type = IMAPCommandType::ICT_LOGIN;
				else if (command == "list")
					this->c_Type = IMAPCommandType::ICT_LIST;
				else
					this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			case 's':
			{
				if (command == "starttls")
					this->c_Type = IMAPCommandType::ICT_STARTTLS;
				else
					this->c_Type = IMAPCommandType::ICT_UNKNOWN;
				break;
			}
			default: this->c_Type = IMAPCommandType::ICT_UNKNOWN;
		}

		// Gets the arguments, and removes the first whitespace
		std::string argumentsRaw = other.substr(cmdArgSep);
		if (argumentsRaw[0] == ' ') argumentsRaw.erase(0, 1);

		// Checks if there is anything, if so start parsing
		if (!argumentsRaw.empty())
		{
			std::stringstream stream(argumentsRaw);
			std::string arg;
			while (std::getline(stream, arg, ' '))
				this->c_Args.push_back(arg);
		}
	}
}