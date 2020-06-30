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

#include "Command.src.h"

namespace FSMTP::SMTP
{
	ClientCommand::ClientCommand():
		c_CommandType(ClientCommandType::CCT_UNKNOWN), c_Arguments()
	{

	}

	ClientCommand::ClientCommand(const ClientCommandType &c_CommandType, const std::vector<std::string> &c_Arguments):
		c_CommandType(c_CommandType), c_Arguments(c_Arguments)
	{

	}

	ClientCommand::ClientCommand(const std::string &raw)
	{
		// Parses the command from the raw message
		// - Sometimes it does not contain an ':' and then
		// - we search for an space
		std::size_t index = raw.find_first_of(':');
		std::string command;
		bool containsArgs = false;
		if (index != std::string::npos)
		{
			containsArgs = true;
			command = raw.substr(0, index);
		} else
		{
			index = raw.find_first_of(' ');
			if (index == std::string::npos) command = raw;
			else
			{
				containsArgs = true;
				command = raw.substr(0, index);
			}
		}

		// Converts the command copy to an lower string
		// - so nothing can go wrong when comparing it
		auto funcToLower = [](unsigned char c){
			return std::tolower(c);
		};
		std::transform(command.begin(), command.end(), command.begin(), funcToLower);

		// Checks which command it is, and due to performance
		// - we first switch the initial character, and then perform
		// - specific compare operations
		switch (command[0])
		{
			case 'h':
			{
				if (command.substr(0, 4) == "helo" || command.substr(0, 4) == "ehlo")
					this->c_CommandType = ClientCommandType::CCT_HELO;
				else 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
			}
			case 's':
			{
				if (command.substr(0, 8) == "starttls")
					this->c_CommandType = ClientCommandType::CCT_START_TLS;
				else 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
			}
			case 'm':
			{
				if (command.substr(0, 9) == "mail from")
					this->c_CommandType = ClientCommandType::CCT_MAIL_FROM;
				else 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
			}
			case 'r':
			{
				if (command.substr(0, 8) == "rcpt to")
					this->c_CommandType = ClientCommandType::CCT_RCPT_TO;
				else 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
			}
			case 'd':
			{
				if (command.substr(0, 4) == "data")
					this->c_CommandType = ClientCommandType::CCT_DATA;
				else 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
			}
			case 'q':
			{
				if (command.substr(0, 4) == "quit")
					this->c_CommandType = ClientCommandType::CCT_QUIT;
				else 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
			}
			default:
			{
				this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
			}
		}

		// Checks if we may have arguments, if so it will start parsing
		// - them, maybe with a custom parser if required
		if (containsArgs == false) return;
		std::string argsRaw = raw.substr(++index);
		std::cout << argsRaw << std::endl;
	}

	void ClientCommand::build(std::string &ret)
	{
		// Adds the command name and later the parameters
		switch (this->c_CommandType)
		{
			case ClientCommandType::CCT_HELO:
			{
				ret += "HELO";
				break;
			}
			case ClientCommandType::CCT_START_TLS:
			{
				ret += "STARTTLS";
				break;
			}
			case ClientCommandType::CCT_MAIL_FROM:
			{
				ret += "MAIL FROM";
				break;
			}
			case ClientCommandType::CCT_RCPT_TO:
			{
				ret += "RCPT TO";
				break;
			}
			case ClientCommandType::CCT_DATA:
			{
				ret += "DATA";
				break;
			}
			case ClientCommandType::CCT_QUIT:
			{
				ret += "QUIT";
				break;
			}
		}

		for (const std::string &s : this->c_Arguments)
		{
			ret += ' ';
			ret += s;
		}
		ret += "\r\n";
	}
}
