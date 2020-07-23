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

/**
 * Application entry LOL, idiots.. Jk jk
 *
 * @Param {const int} argc
 * @Param {const char **} argv
 * @Return {int}
 */
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
		
			for (;;)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(120));
			};
		}
		case ServerType::ST_SMTP:
		{
			Logger logger("Main", LoggerLevel::INFO);
			logger << WARN << "Fannst SMTP/ESMTP Server door Luke A.C.A. Rieff, vrij onder de Apache 2.0 license" << ENDL << CLASSIC;

			// Creates and starts the database worker
			std::unique_ptr<DatabaseWorker> dbWorker = std::make_unique<DatabaseWorker>();
			if (!dbWorker->start(nullptr))
				std::exit(-1);
			// Runs the transmission worker
			std::unique_ptr<TransmissionWorker> transWorker = std::make_unique<TransmissionWorker>(_CASSANDRA_DATABASE_CONTACT_POINTS);
			if (!transWorker->start(nullptr))
				std::exit(-1);
			// Runs the server
			SMTPServer server(25, true);

			// Loops forever
			for (;;)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(120));\
			};
		}
		case ServerType::ST_IMAP:
		{
			Logger logger("Main", LoggerLevel::INFO);
			logger << WARN << "Fannst IMAP Server door Luke A.C.A. Rieff, vrij onder de Apache 2.0 license" << ENDL << CLASSIC;

			IMAP::IMAPServer sock(143, 993);

			for (;;)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(120));
			};
		}
		default: return -1;
	}

	return 0;
}
