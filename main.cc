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
extern Json::Value _config;

/**
 * Application entry LOL, idiots.. Jk jk
 *
 * @Param {const int} argc
 * @Param {const char **} argv
 * @Return {int}
 */
int main(const int argc, const char **argv)
{
	// Loads the configuration, and initializes OpenSSL
	Configuration::read("../config.json");
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
			POP3::P3Server pop3server(_config["ports"]["pop3_plain"].asInt());
		
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
			std::unique_ptr<TransmissionWorker> transWorker = std::make_unique<TransmissionWorker>(_config["database"]["cassandra_hosts"].asCString());
			if (!transWorker->start(nullptr))
				std::exit(-1);
			// Runs the server
			SMTPServer server(_config["ports"]["smtp_plain"].asInt(), true);

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

			IMAP::IMAPServer sock(
				_config["ports"]["imap_plain"].asInt(),
				_config["ports"]["imap_secure"].asInt()
			);

			for (;;)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(120));
			};
		}
		case ServerType::ST_DNS:
		{
			Logger logger("Main", LoggerLevel::INFO);
			logger << WARN << "Fannst DNS Server door Luke A.C.A. Rieff, vrij onder de Apache 2.0 license" << ENDL << CLASSIC;
			
			try
			{
				DNS::DNSServer dnsServer(_config["ports"]["dns_gen"].asInt());
				
				for (;;)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(120));
				};
			} catch (const SocketInitializationException &e)
			{
				logger << FATAL << e.what() << ENDL << CLASSIC;
			}
		}
		default: return -1;
	}

	return 0;
}
