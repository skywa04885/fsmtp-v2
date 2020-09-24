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

namespace FSMTP
{
	vector<CMDArg> CMDArg::parse(const vector<string> &raw) {
		vector<CMDArg> res = {};

		size_t i = 0;
		for (const string &arg : raw) {
			if (i++ == 0) {
				continue;
			}

			size_t index = arg.find_first_of('=');
			if (index == string::npos) {
				res.push_back(CMDArg(arg, ""));
				continue;
			}

			res.push_back(CMDArg(arg.substr(0, index), arg.substr(index+1)));
		}

		return res;
	}

	CMDArg::CMDArg(
	    const string &c_Name,
	    const string &c_Arg
	):
		c_Name(c_Name), c_Arg(c_Arg)
	{}

	CMDArg::CMDArg(void) {}

	bool CMDArg::compare(const string &command) {
		string clean;
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

	void handleArguments(const vector<string> &argList) {
		vector<CMDArg> arguments = CMDArg::parse(argList);

		for (CMDArg &arg : arguments)
		{
			if (arg.compare("test")) ARG_ACTIONS::testArgAction();
			else if (arg.compare("mailtest")) ARG_ACTIONS::mailTestArgAction();
			else if (arg.compare("domainadd")) ARG_ACTIONS::addDomain();
			else if (arg.compare("adduser")) ARG_ACTIONS::addUser();

			if (arg.compare("help"))
			{
				cout << "Gebruik: " << endl;
				cout << "sudo fsmtp [arguments]" << endl;

				cout << endl << "Opdrachten: " << endl;
				cout << "-h, -help: " << "\tPrint de lijst met beschikbare opdrachten." << endl;
				cout << "-t, -test: " << "\tVoer tests uit op de vitale functies van de server, zoals database verbinding." << endl;
				cout << "-s, -sync: " << "\tSynchroniseerd de redis database met die van cassandra" << endl;
				cout << "-a, -adduser: " << "\tAdds an user to the database" << endl;
				cout << "-d, -domainadd:" << "\tAdds an new domain." << endl;
				cout << "-m, -mailtest: " << "\tSends an email." << endl;
				cout << "-r, -run=type: " << "\tWelke server er gestart moet worden, 'smtp' of 'pop3'" << endl;

				exit(0);
			}
		}
	}
}
