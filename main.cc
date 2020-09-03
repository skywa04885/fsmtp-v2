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
#include "lib/dkim/DKIMVerifier.src.h"
#include "lib/smtp/server/SMTPSpamDetection.src.h"

static const char *CONFIG_FILE = "../config.json";
static const char *FALLBACK_CONFIG_FILE = "../fallback/config.json";

int main(const int argc, const char **argv)
{
	ifstream file;
	file.open("../gitlab.eml");
	if (!file.is_open())
		throw runtime_error("Could not open file");

	string line, total;
	while (getline(file, line)) {
		total += line;
		total += '\n';
	}

	DKIM_Verifier::verify(total);

	return 0;

	// ==================================
	// Default main
	// ==================================

	Logger logger("MAIN", LoggerLevel::INFO);

	Global::configure();
	Global::readConfig(CONFIG_FILE, FALLBACK_CONFIG_FILE);

	vector<string> args(argv, argv + argc);
	handleArguments(args);

	// ==================================
	// Starts the services
	// ==================================

	Server::SMTPServer smtpServer;
	POP3::P3Server pop3Server;
	Workers::TransmissionWorker transmissionWorker;
	Workers::DatabaseWorker databaseWorker;

	try {
		pop3Server
			.createContext()
			.connectDatabases()
			.listenServer()
			.startHandler(true);
	} catch (const runtime_error  &e) {
		logger << FATAL << "Could not start service POP3 server";
		logger << ", error: " << e.what() << ENDL << CLASSIC;
	}

	try {
		smtpServer
			.createContext()
			.connectDatabases()
			.listenServer()
			.startHandler(true);
	} catch (const runtime_error  &e) {
		logger << FATAL << "Could not start service ESMTP server";
		logger << ", error: " << e.what() << ENDL << CLASSIC;
	}

	transmissionWorker.start(nullptr);
	databaseWorker.start(nullptr);

	for (;;) {
		this_thread::sleep_for(seconds(1));
	}

	return 0;
}
