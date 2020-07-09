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

#include "main.h"

int main(const int argc, const char **argv)
{
	// Parses the arguments into an vector after this
	// - we define the anonymous function which will compare
	// - an string and an argument, this will allow all the possible
	// - formats such as -<A> --<A> -<A[0]>,
	// - after this we start an loop which checks for specific args
	std::vector<std::string> args(argv, argv+argc);
	auto compareArg = [](const std::string &a, const std::string &b)
	{
		if (a == "--" + b || a == "-" + b.substr(0, 1)) return true;
		else return false;	
	};
	for (const std::string &arg : args)
	{
		if (compareArg(arg, "help"))
		{
			std::cout << "Gebruik: " << std::endl;
			std::cout << "sudo fsmtp [arguments]" << std::endl;

			std::cout << std::endl << "Opdrachten: " << std::endl;
			std::cout << "-h, --help: " << "\tPrint de lijst met beschikbare opdrachten." << std::endl;
			std::cout << "-t, --test: " << "\tVoer tests uit op de vitale functies van de server, zoals database verbinding." << std::endl;
			return 0;
		}

		if (compareArg(arg, "test"))
		{
			// Creates the logger and starts performing the tests
			Logger logger("test", LoggerLevel::INFO);
			logger << "FSMTP gaat nu beginnen met testen !" << ENDL;

			// Performs the database connection tests, so we 
			// - can check if we may reach apache cassandra
			logger << _BASH_UNKNOWN_MARK << "Start van Apache Cassandra || Datastax Enterprise database verbindings test ..." << ENDL;
			try { CassandraConnection connection(_CASSANDRA_DATABASE_CONTACT_POINTS); }
			catch (const std::runtime_error &e)
			{
				// Closes since the database connections messes everything up if we wont
				return -1;
			}
			logger << _BASH_SUCCESS_MARK << "Verbinding met Apache Cassandra || Datastax Enterprise is geslaagd !" << ENDL;

			// Performs the check if we may create an socket
			// - this will throw an error if we're not allowed to do it
			logger << _BASH_UNKNOWN_MARK << "Start van server socket test ..." << ENDL;
			try { SMTPSocket socket(SMTPSocketType::SST_SERVER, 25); }
			catch (const std::runtime_error &e)
			{
				logger << _BASH_FAIL_MARK << "Het starten en stoppen van een server socket is fout gegaan, probeer de server met 'sudo' te starten." << ENDL;
			}
			logger << _BASH_SUCCESS_MARK << "Het starten en stoppen van een server socket is geslaagd !" << ENDL;

			// Closes the application since the tests are finished
			return 0;
		}
	}

	// Runs the server
	Logger logger("Main", LoggerLevel::INFO);

	// Prints the credits haha
	for (std::size_t i = 0; i < 80; i++)
		logger << '=';
	logger << ENDL;
	logger << "FSMTP V2 Door Skywa04885 - https://software.fannst.nl/fsmtp-v2" << ENDL;
	for (std::size_t i = 0; i < 80; i++)
		logger << '=';
	logger << ENDL;

	int32_t opts = 0x0;

	opts |= _SERVER_OPT_ENABLE_AUTH;
	opts |= _SERVER_OPT_ENABLE_TLS;

	SMTPServer server(25, true, opts);

	std::cin.get();
	server.shutdownServer();

	return 0;
}
