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
	{}

	ClientCommand::ClientCommand(const ClientCommandType c_CommandType, const vector<string> &c_Arguments):
		c_CommandType(c_CommandType), c_Arguments(c_Arguments)
	{}
	
	ClientCommand::ClientCommand(const string &raw) {
		// Parses the command from the raw message
		// - Sometimes it does not contain an ':' and then
		// - we search for an space
		size_t index = raw.find_first_of(':');
		string command;
		bool containsArgs = false;
		if (index != string::npos)
		{
			containsArgs = true;
			command = raw.substr(0, index);
		} else
		{
			index = raw.find_first_of(' ');
			if (index == string::npos) command = raw;
			else
			{
				containsArgs = true;
				command = raw.substr(0, index);
			}
		}

		// Converts the command copy to an lower string
		// - so nothing can go wrong when comparing it
		auto funcToLower = [](unsigned char c){
			return tolower(c);
		};
		transform(command.begin(), command.end(), command.begin(), funcToLower);

		// Checks which command it is, and due to performance
		// - we first switch the initial character, and then perform
		// - specific compare operations
		auto &type = this->c_CommandType;
		switch (command[0])
		{
			case 'h':
				if (command == "helo") type = ClientCommandType::CCT_HELO;
				else if (command == "help") type = ClientCommandType::CCT_HELP;
				else type = ClientCommandType::CCT_UNKNOWN;
				break;
			case 'e':
				if (command == "ehlo") type = ClientCommandType::CCT_EHLO;
				else type = ClientCommandType::CCT_UNKNOWN;
				break;
			case 's':
				if (command == "starttls") type = ClientCommandType::CCT_START_TLS;
				else if (command == "su") type = ClientCommandType::CCT_SU;
				else type = ClientCommandType::CCT_UNKNOWN;
				break;
			case 'm':
				if (command == "mail from") type = ClientCommandType::CCT_MAIL_FROM;
				else type = ClientCommandType::CCT_UNKNOWN;
				break;
			case 'r':
				if (command == "rcpt to") type = ClientCommandType::CCT_RCPT_TO;
				else type = ClientCommandType::CCT_UNKNOWN;
				break;
			case 'd':
				if (command == "data") type = ClientCommandType::CCT_DATA;
				else type = ClientCommandType::CCT_UNKNOWN;
				break;
			case 'q':
				if (command == "quit") type = ClientCommandType::CCT_QUIT;
				else type = ClientCommandType::CCT_UNKNOWN;
				break;
			case 'a':
				if (command == "auth") this->c_CommandType = ClientCommandType::CCT_AUTH;
				else this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
			case 'f':
				if (command == "fcapa") this->c_CommandType = ClientCommandType::CCT_FCAPA;
				else this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
			default:
				this->c_CommandType = ClientCommandType::CCT_UNKNOWN;
				break;
		}

		// Checks if we may have arguments, if so it will start parsing
		// - them, maybe with a custom parser if required.. After that
		// - we remove all the double whitespace, to make it more easy
		// - for later processing
		if (containsArgs == false) {
			return;
		}
		string argsRaw = raw.substr(++index);
		string ret;
		reduceWhitespace(argsRaw, ret);
		if (!ret.empty()) {
			stringstream stream(ret);
			string arg;
			while (getline(stream, arg, ' ')) {
				this->c_Arguments.push_back(arg);
			}
		}
	}

	string ClientCommand::build(void) {
		ostringstream stream;

		switch (this->c_CommandType) {
			case ClientCommandType::CCT_HELO:
				stream << "HELO";
				break;
			case ClientCommandType::CCT_EHLO:
				stream << "EHLO";
				break;
			case ClientCommandType::CCT_START_TLS:
				stream << "STARTTLS";
				break;
			case ClientCommandType::CCT_MAIL_FROM:
				stream << "MAIL FROM";
				break;
			case ClientCommandType::CCT_RCPT_TO:
				stream << "RCPT TO";
				break;
			case ClientCommandType::CCT_DATA:
				stream << "DATA";
				break;
			case ClientCommandType::CCT_QUIT:
				stream << "QUIT";
				break;
			case ClientCommandType::CCT_AUTH:
				stream << "AUTH";
				break;
			case ClientCommandType::CCT_HELP:
				stream << "HELP";
				break;
			case ClientCommandType::CCT_SU:
				stream << "SU";
				break;
			case ClientCommandType::CCT_FCAPA:
				stream << "SU";
				break;
		}

		auto &type = this->c_CommandType;
		if (type == ClientCommandType::CCT_MAIL_FROM || type == ClientCommandType::CCT_RCPT_TO) {
			stream << ':';
		}

		auto &args = this->c_Arguments;
		for_each(args.begin(), args.end(), [&](auto &arg) {
			stream << ' ' << arg;
		});
		stream << "\r\n";

		return stream.str();
	}
}
