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

#include "args.src.h"

extern bool _forceLoggerNCurses;
extern ServerType _serverType;

namespace FSMTP
{
	/**
	 * Parses an raw string of arguments into an argument
	 * - vector
	 *
	 * @Param {const std::vector<std::string> &} raw
	 * @Return {std::vector<CMDArg>}
	 */
	std::vector<CMDArg> CMDArg::parse(
		const std::vector<std::string> &raw
	)
	{
		std::vector<CMDArg> res = {};

		// Loops over the arguments and parses them
		std::size_t i = 0;
		for (const std::string &arg : raw)
		{
			if (i++ == 0) continue;

			// Gets the value of index, and appends
			// - the current string if it is not there
			std::size_t index = arg.find_first_of('=');
			if (index == std::string::npos)
			{
				res.push_back(CMDArg(arg, ""));
				continue;
			}

			// Pushes the key with the value to the result vector
			res.push_back(CMDArg(arg.substr(0, index), arg.substr(index+1)));
		}

		// Returns the vector
		return res;
	}

	/**
	 * Default constructor for an command line argument
	 *
	 * @Param {const std::String &} c_Name
	 * @Param {const std::vector<std::string &} c_Arg
	 */
	CMDArg::CMDArg(
	    const std::string &c_Name,
	    const std::string &c_Arg
	):
		c_Name(c_Name), c_Arg(c_Arg)
	{}

	/**
	 * The empty default constructor for an command line
	 * - argument
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	CMDArg::CMDArg(void)
	{}

	/**
	 * Checks if the current command matches the
	 * - specified command
	 *
	 * @Param {const std::string &} command
	 * @Return {bool}
	 */
	bool CMDArg::compare(const std::string &command)
	{
		std::string clean;
		if (this->c_Name[0] == '-')
			clean = this->c_Name.substr(1);
		else
			clean = this->c_Name;

		if (command[0] == clean[0])
			return true;
		else if (clean == command)
			return true;
		else
			return false;
	}

	/**
	 * Handles the arguments from the FSMTP program
	 *
	 * @Param {const std::vector<std::string> &} argList
	 * @Return {void}
	 */
	void handleArguments(const std::vector<std::string> &argList)
	{
		Logger logger("ArgHandler", LoggerLevel::INFO);
		std::vector<CMDArg> arguments = CMDArg::parse(argList);

		// Checks if there are any arguments, and if so
		// - print them all
		if (arguments.size() > 0)
		{
			logger << "Found " << arguments.size() << " arguments .." << ENDL;
			std::size_t i = 0;
			for (const CMDArg &arg : arguments)
				logger << "Argument [" << i++ << "]: <" << arg.c_Name << "> - <" << arg.c_Arg << '>' << ENDL;
		}

		// Loops over the arguments and tries to bind
		// - an action to it, and then call the function
		// - which is bound to it
		for (CMDArg &arg : arguments)
		{
			if (arg.compare("test"))
				ARG_ACTIONS::testArgAction();

			if (arg.compare("adduser"))
				ARG_ACTIONS::addUserArgAction();

			if (arg.compare("sync"))
				ARG_ACTIONS::syncArgAction();

			if (arg.compare("mailtest"))
				ARG_ACTIONS::mailTestArgAction();

			if (arg.compare("help"))
			{
				std::cout << "Gebruik: " << std::endl;
				std::cout << "sudo fsmtp [arguments]" << std::endl;

				std::cout << std::endl << "Opdrachten: " << std::endl;
				std::cout << "-h, -help: " << "\tPrint de lijst met beschikbare opdrachten." << std::endl;
				std::cout << "-t, -test: " << "\tVoer tests uit op de vitale functies van de server, zoals database verbinding." << std::endl;
				std::cout << "-s, -sync: " << "\tSynchroniseerd de redis database met die van cassandra" << std::endl;
				std::cout << "-a, -adduser:" << "\tAdds an new user to the email server." << std::endl;
				std::cout << "-m, -mailtest: " << "\tSends an email." << std::endl;
				std::cout << "-r, -run=type: " << "\tWelke server er gestart moet worden, 'smtp' of 'pop3'" << std::endl;

				std::exit(0);
			}

			if (arg.compare("run"))
			{
				if (arg.c_Arg == "smtp")
					_serverType = ServerType::ST_SMTP;
				if (arg.c_Arg == "pop3")
					_serverType = ServerType::ST_POP3;
				if (arg.c_Arg == "imap")
					_serverType = ServerType::ST_IMAP;
				if (arg.c_Arg == "dns")
					_serverType = ServerType::ST_DNS;
			}
		}
	}
}
