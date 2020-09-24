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
#include "lib/spam/NaiveBayes.src.h"
#include "lib/spam/WordEncoder.src.h"

using namespace FSMTP::Spam::Preprocessing;

static const char *CONFIG_FILE = "../config.json";
static const char *FALLBACK_CONFIG_FILE = "../fallback/config.json";

int main(const int argc, const char **argv)
{
	// Logger l("MAIN", LoggerLevel::INFO);

	// vector<string> stopWords = {};
	// {
	// 	Json::Value temp;
	// 	ifstream file("../datasets/EN_STOPWORDS.json", ios::in);
	// 	DEFER(file.close());
	// 	file >> temp;

	// 	Json::Value defaultValue;
	// 	for (size_t i = 0; i < temp.size(); ++i) {
	// 		Json::Value row = temp.get(i, defaultValue);
	// 		stopWords.push_back(row.asString());
	// 	}
	// }

	// // Reads the json file with the spam and ham messages
	// //  into the memory, which will be parsed later on
	// Json::Value spamHam;
	// ifstream file("../datasets/EN_SMS-SPAM.json", ios::in);
	// DEFER(file.close());
	// file >> spamHam;

	// // Starts looping over the rows, and pushing the messages
	// //  into the according vectors
	// vector<string> spamMessages = {};
	// vector<string> hamMessages = {};

	// Json::Value defaultValue;
	// for (size_t i = 0; i < spamHam.size(); ++i) {
	// 	Json::Value row = spamHam.get(i, defaultValue);
	// 	if (row["class"].asString() == "ham") hamMessages.push_back(row["value"].asString());
	// 	else spamMessages.push_back(row["value"].asString());
	// }

	// // Starts joining the messages, this will be used to count the
	// //  words for each category, and create the histogram
	// string spam, ham;
	// for_each(spamMessages.begin(), spamMessages.end(), [&](const string &a) { spam += a + '\n'; });
	// for_each(hamMessages.begin(), hamMessages.end(), [&](const string &a) { ham += a + '\n'; });

	// // Parses some stuff
	// vector<string> words = {};
	// WordEncoder::splitString(spam, words);

	// WordEncoder encoder;
	// encoder.fit(words, [&](const string &str) {
	// 	if (str.length() < 3) return false;
	// 	else if (find(stopWords.begin(), stopWords.end(), str) != stopWords.end()) return false;
	// 	else return true;
	// }).print(l);

	// return 0;

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
	unique_ptr<Workers::TransmissionWorker> transmissionWorker = make_unique<Workers::TransmissionWorker>();
	unique_ptr<Workers::DatabaseWorker> databaseWorker = make_unique<Workers::DatabaseWorker>();

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

	transmissionWorker->start(nullptr);
	databaseWorker->start(nullptr);

	for (;;) {
		this_thread::sleep_for(seconds(1));
	}

	return 0;
}
