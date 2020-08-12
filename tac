












		
			
				
				{
				};
			);
			{
			{
			{
			{
			{
			}
			};
			};
			};
		{
		{
		{
		{
		}
		}
		}
		}
	{
	}
 *
 */
*/
/*
/**
{
}
 * Application entry LOL, idiots.. Jk jk
		case ServerType::ST_DNS:
		case ServerType::ST_IMAP:
		case ServerType::ST_POP3:
		case ServerType::ST_SMTP:
			} catch (const SocketInitializationException &e)
				_config["ports"]["imap_plain"].asInt(),
				_config["ports"]["imap_secure"].asInt()
	Configuration::read("../config.json");
	Copyright [2020] [Luke A.C.A. Rieff]
			// Creates and starts the database worker
		default: return -1;
	distributed under the License is distributed on an "AS IS" BASIS,
				DNS::DNSServer dnsServer(_config["ports"]["dns_gen"].asInt());
extern Json::Value _config;
				for (;;)
			for (;;)
			for (;;)
			for (;;)
	handleArguments(argList);
	// Handles the arguments
	http://www.apache.org/licenses/LICENSE-2.0
			if (!dbWorker->start(nullptr))
			if (!transWorker->start(nullptr))
			IMAP::IMAPServer sock(
#include "main.h"
int main(const int argc, const char **argv)
	Licensed under the Apache License, Version 2.0 (the "License");
	limitations under the License.
	// Loads the configuration, and initializes OpenSSL
				logger << FATAL << e.what() << ENDL << CLASSIC;
			Logger logger("Main", LoggerLevel::INFO);
			Logger logger("Main", LoggerLevel::INFO);
			Logger logger("Main", LoggerLevel::INFO);
			Logger logger("Main", LoggerLevel::INFO);
			logger << WARN << "Fannst DNS Server door Luke A.C.A. Rieff, vrij onder de Apache 2.0 license" << ENDL << CLASSIC;
			logger << WARN << "Fannst IMAP Server door Luke A.C.A. Rieff, vrij onder de Apache 2.0 license" << ENDL << CLASSIC;
			logger << WARN << "Fannst POP3 Server door Luke A.C.A. Rieff, vrij onder de Apache 2.0 license" << ENDL << CLASSIC;
			logger << WARN << "Fannst SMTP/ESMTP Server door Luke A.C.A. Rieff, vrij onder de Apache 2.0 license" << ENDL << CLASSIC;
			// Loops forever
	OpenSSL_add_ssl_algorithms();
 * @Param {const char **} argv
 * @Param {const int} argc
			POP3::P3Server pop3server(_config["ports"]["pop3_plain"].asInt());
	return 0;
 * @Return {int}
			// Runs the server
			// Runs the transmission worker
	See the License for the specific language governing permissions and
ServerType _serverType = ServerType::ST_SMTP;
			SMTPServer server(_config["ports"]["smtp_plain"].asInt(), true);
	SSL_load_error_strings();
				std::exit(-1);
				std::exit(-1);
					std::this_thread::sleep_for(std::chrono::milliseconds(120));
				std::this_thread::sleep_for(std::chrono::milliseconds(120));
				std::this_thread::sleep_for(std::chrono::milliseconds(120));
				std::this_thread::sleep_for(std::chrono::milliseconds(120));\
			std::unique_ptr<DatabaseWorker> dbWorker = std::make_unique<DatabaseWorker>();
			std::unique_ptr<TransmissionWorker> transWorker = std::make_unique<TransmissionWorker>(_config["database"]["cassandra_hosts"].asCString());
	std::vector<std::string> argList(argv, argv+argc);
	switch (_serverType)
			try
	Unless required by applicable law or agreed to in writing, software
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at
