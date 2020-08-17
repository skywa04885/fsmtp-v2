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

#include "Logger.src.h"

using namespace FSMTP;

void Logger::saveToDisk(const string &message) {
	char filename[128];
	time_t rawTime;
	struct tm *timeInfo = nullptr;

	// Checks if the logs dir exists, if not create the dir
	if (!filesystem::exists("../logs")) {
		filesystem::create_directory("../logs");
	}

	// Appends the message to the final logs, the file name will be generated
	//  with the strftime and the current date
	
	time(&rawTime);
	timeInfo = localtime(&rawTime);
	strftime(filename, sizeof(filename), "../logs/%a, %d %b %Y.txt", timeInfo);
	
	ofstream stream(filename, ios::app);
	DEFER(stream.close());
	stream << message << endl;
}
