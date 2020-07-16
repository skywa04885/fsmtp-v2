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

#include "P3Commands.src.h"

namespace FSMTP::POP3
{
	void P3Command::parse(const std::string &raw)
	{
		// Cleans the message, so it's easier to work with
		std::string clean;
		reduceWhitespace(raw, clean);
		removeFirstAndLastWhite(clean);
		std::transform(clean.begin(), clean.end(), clean.begin(), [](unsigned char c){
			return std::tolower(c);
		});

		// Separates the command from the rest of the crap
		std::string command;
		std::size_t index = clean.find_first_of(' ');
		if (index == std::string::npos)
			command = clean;
		else command = clean.substr(0, index);

		// Checks which command it is
		switch (command[0])
		{
			case 'c':
			{
				if (command == "capa")
					this->c_Type = POP3CommandType::PCT_CAPA;
				else
					this->c_Type = POP3CommandType::PCT_UNKNOWN;
				break;
			}
			case 'q':
			{
				if (command == "quit")
					this->c_Type = POP3CommandType::PCT_QUIT;
				else
					this->c_Type = POP3CommandType::PCT_UNKNOWN;
			}
		}

		// Gets the arguments
		if (index != std::string::npos)
		{
			std::string args = clean.substr(++index);
			std::stringstream stream(args);
			std::string arg;
			while (std::getline(stream, arg, ' '))
			{
				this->c_Args.push_back(arg);
			}
		}
	}
}