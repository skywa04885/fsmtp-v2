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

#pragma once

#include "../default.h"

namespace FSMTP {
	typedef enum : uint8_t {
		DEBUG = 0,
		PARSER,
		INFO,
		WARN,
		ERROR,
		FATAL
	} LoggerLevel;

	typedef enum : uint8_t {
		ENDL = 0,
		CLASSIC,
		FLUSH
	} LoggerOpts;

	class Logger {
	public:
		Logger(const string &l_Prefix, const LoggerLevel &l_Level):
			l_Prefix(l_Prefix), l_Level(l_Level)
		{}

		static void saveToDisk(const string &message);

		template<typename T>
		Logger &append(const T &a) {
			this->l_Stream << a;
			return *this;
		}

		Logger &append(const LoggerLevel& a) {
			this->l_Old = this->l_Level;
			this->l_Level = a;

			return *this;
		}

		Logger &append(const LoggerOpts &a) {
			switch (a)
			{
				case LoggerOpts::CLASSIC:
					this->l_Level = this->l_Old;
					break;
				case LoggerOpts::ENDL:
				case LoggerOpts::FLUSH: {
					ostringstream oss;

					char dateBuffer[64];
					time_t rawTime;
					struct tm *timeInfo = nullptr;

					time(&rawTime);
					timeInfo = localtime(&rawTime);
					strftime(dateBuffer, sizeof(dateBuffer), "%a, %d %b %Y %T", timeInfo);
					cout << dateBuffer << "->";

					// Appends the level, and checks if the message should be logged to
					//  the file, this only happens on an error.

					switch (this->l_Level) {
						case LoggerLevel::DEBUG:
							oss << "\033[36m(DEBUG@" << this->l_Prefix << "): \033[0m";
							break;
						case LoggerLevel::PARSER:
							oss << "\033[34m(PARSER@" << this->l_Prefix << "): \033[0m";
							break;
						case LoggerLevel::INFO:
							oss << "\033[32m(INFO@" << this->l_Prefix << "): \033[0m";
							break;
						case LoggerLevel::WARN:
							oss << "\033[33m(WARN@" << this->l_Prefix << "): \033[0m";
							break;
						case LoggerLevel::ERROR:
							oss << "\033[31m(ERR@" << this->l_Prefix << "): \033[0m";
							break;
						case LoggerLevel::FATAL:
							oss << "\033[31m[FATAL@" << this->l_Prefix << "]: \033[0m";
							break;
					}

					oss << this->l_Stream.str();
					if (this->l_Level == LoggerLevel::FATAL || this->l_Level == LoggerLevel::ERROR) {
						Logger::saveToDisk(oss.str());
					}

					// Prints the message with the specified method, after this
					//  we clear the internal buffer

					if (a == LoggerOpts::FLUSH) {
						cout << oss.str() << flush;
					} else {
						cout << oss.str() << endl;
					}

					this->l_Stream.str("");
					this->l_Stream.clear();
					break;
				}
			}

			return *this;
		}

		template<typename T>
		Logger &operator << (const T &a) {
			return this->append(a);
		}
	private:
		ostringstream l_Stream;
		string l_Prefix;
		LoggerLevel l_Level;
		LoggerLevel l_Old;
	};
}