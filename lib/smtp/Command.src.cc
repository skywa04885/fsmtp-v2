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
	/**
	 * Default empty constructor for the ClientCommand
	 *
	 * @Param void
	 * @Return void
	 */
	ClientCommand::ClientCommand():
		c_CommandType(ClientCommandType::CCT_UNKNOWN), c_Arguments()
	{
	}

	/**
	 * Default constructor for the ClientCommand
	 *
	 * @Param {ClientCommandType &} c_CommandType
	 * @Param {std::vector<std::string> &} c_Arguments
	 * @Return void
	 */
	ClientCommand::ClientCommand(const ClientCommandType &c_CommandType, const std::vector<std::string> &c_Arguments):
		c_CommandType(c_CommandType), c_Arguments(c_Arguments)
	{

	}
	
	/**
	 * Default constructor which actually
	 * - parses an existing command
	 *
	 * @Param {std::string &} raw
	 * @Return void
	 */
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
				if (command == "helo") {
					this->c_CommandType = ClientCommandType::CCT_HELO;
				} else if (command == "help") {
					this->c_CommandType = ClientCommandType::CCT_HELP;
				} else { 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				}
				break;
			case 'e':
				if (command == "ehlo") {
					this->c_CommandType = ClientCommandType::CCT_EHLO;
				} else { 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				}
				break;
			case 's':
				if (command == "starttls") {
					this->c_CommandType = ClientCommandType::CCT_START_TLS;
				} else {
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				}
				break;
			case 'm':
				if (command == "mail from") {
					this->c_CommandType = ClientCommandType::CCT_MAIL_FROM;
				} else {
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				}
				break;
			case 'r':
				if (command == "rcpt to") {
					this->c_CommandType = ClientCommandType::CCT_RCPT_TO;
				} else {
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				}
				break;
			case 'd':
				if (command == "data") {
					this->c_CommandType = ClientCommandType::CCT_DATA;
				} else { 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				}
				break;
			case 'q':
				if (command == "quit") {
					this->c_CommandType = ClientCommandType::CCT_QUIT;
				} else { 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				}
				break;
			case 'a':
				if (command == "auth") {
					this->c_CommandType = ClientCommandType::CCT_AUTH;
				} else { 
					this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				}
				break;
			default:
				this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
		}

		// Checks if we may have arguments, if so it will start parsing
		// - them, maybe with a custom parser if required.. After that
		// - we remove all the double whitespace, to make it more easy
		// - for later processing
		if (containsArgs == false) return;
		std::string argsRaw = raw.substr(++index);
		std::string ret;
		reduceWhitespace(argsRaw, ret);
		if (!ret.empty())
		{
			std::stringstream stream(ret);
			std::string arg;
			while (std::getline(stream, arg, ' '))
			{
				this->c_Arguments.push_back(arg);
			}
		}
	}


	/**
	 * Builds the client command
	 *
	 * @Param {void}
	 * @Return {std::string}
	 */
	std::string ClientCommand::build(void)
	{
		std::string res;

		// Adds the command name and later the parameters
		switch (this->c_CommandType)
		{
			case ClientCommandType::CCT_HELO:
			{
				res += "HELO";
				break;
			}
			case ClientCommandType::CCT_EHLO:
			{
				res += "EHLO";
				break;
			}
			case ClientCommandType::CCT_START_TLS:
			{
				res += "STARTTLS";
				break;
			}
			case ClientCommandType::CCT_MAIL_FROM:
			{
				res += "MAIL FROM";
				break;
			}
			case ClientCommandType::CCT_RCPT_TO:
			{
				res += "RCPT TO";
				break;
			}
			case ClientCommandType::CCT_DATA:
			{
				res += "DATA";
				break;
			}
			case ClientCommandType::CCT_QUIT:
			{
				res += "QUIT";
				break;
			}
			case ClientCommandType::CCT_AUTH:
			{
				res += "AUTH";
				break;
			}
			case ClientCommandType::CCT_HELP:
			{
				res += "HELP";
				break;
			}
		}

		if (
			this->c_CommandType == ClientCommandType::CCT_MAIL_FROM ||
			this->c_CommandType == ClientCommandType::CCT_RCPT_TO
		) res += ':';

		for (const std::string &s : this->c_Arguments)
		{
			res += ' ';
			res += s;
		}
		res += "\r\n";

		return res;
	}
}
