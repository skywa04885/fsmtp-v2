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

ServerType _serverType = ServerType::ST_SMTP;

int main(const int argc, const char **argv)
{
	// Initializes OpenSSL stuff
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();

	// Handles the arguments
	std::vector<std::string> argList(argv, argv+argc);
	handleArguments(argList);

	switch (_serverType)
	{
		case ServerType::ST_POP3:
		{
			Logger logger("Main", LoggerLevel::INFO);
			logger << WARN << "Fannst POP3 Server door Luke A.C.A. Rieff, vrij onder de Apache 2.0 license" << ENDL << CLASSIC;
			POP3::P3Server pop3server(false);
		
			for (;;) continue;

			break;
		}
		case ServerType::ST_SMTP:
		{
			// =====================================
			// Starts the workers
			//
			// Due to memory leaks, certain
			// - tasks will be performed in workers
			// - such as Cassamdra access
			// =====================================

			// Creates and starts the database worker
			std::unique_ptr<DatabaseWorker> dbWorker = std::make_unique<DatabaseWorker>(_CASSANDRA_DATABASE_CONTACT_POINTS);
			if (!dbWorker->start(nullptr))
				std::exit(-1);

			std::unique_ptr<TransmissionWorker> transWorker = std::make_unique<TransmissionWorker>(_CASSANDRA_DATABASE_CONTACT_POINTS);
			if (!transWorker->start(nullptr))
				std::exit(-1);

			// =====================================
			// Creates the SMTP Server itself
			//
			// Sets the config and starts
			// - the smtp server
			// =====================================

			// Runs the server
			Logger logger("Main", LoggerLevel::INFO);

			SMTPServer server(25, true, _REDIS_PORT, _REDIS_CONTACT_POINTS);

			for (;;) continue;

			break;
		}
		case ServerType::ST_IMAP:
		{
			IMAP::IMAPServer sock(143, 993);

			for (;;) {std::this_thread::sleep_for(std::chrono::milliseconds(120));};
			break;
		}
		default: return -1;
	}

	return 0;
}
