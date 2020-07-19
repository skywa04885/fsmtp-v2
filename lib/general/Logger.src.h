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

#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <cstdint>
#include <chrono>
#include <ctime>

namespace FSMTP
{

	typedef enum : uint8_t
	{
		DEBUG = 0,
		PARSER,
		INFO,
		WARN,
		ERROR,
		FATAL
	} LoggerLevel;

	typedef enum : uint8_t
	{
		ENDL = 0,
		CLASSIC,
		FLUSH
	} LoggerOpts;

	class Logger
	{
	public:
		/**
		 * Default constructor for the logger
		 *
		 * @Param {const std::string &} l_Prefix
		 * @Param {const LoggerLevel &} l_Level
		 * @Return {void}
		 */
		Logger(const std::string &l_Prefix, const LoggerLevel &l_Level):
			l_Prefix(l_Prefix), l_Level(l_Level)
		{}

		/**
		 * Appends something to the stream
		 *
		 * @Param {T &} a
		 * @Return {Logger &}
		 */
		template<typename T>
		Logger &append(const T &a)
		{
			this->l_Stream << a;
			return *this;
		}

		/**
		 * Changes the logger level
		 *
		 * @Param {const LoggerLevel &} a
		 * @Return {Logger &}
		 */
		Logger &append(const LoggerLevel& a)
		{
			this->l_Old = this->l_Level;
			this->l_Level = a;

			return *this;
		}

		/**
		 * Performs an operation such as print out
		 *
		 * @Param {const LoggerOpts &} a
		 * @Return {Logger &}
		 */
		Logger &append(const LoggerOpts &a)
		{
			// Checks which option it is
			switch (a)
			{
				case LoggerOpts::CLASSIC:
				{
					this->l_Level = this->l_Old;
					break;
				}
				case LoggerOpts::ENDL:
				case LoggerOpts::FLUSH:
				{
					// Builds the time and appends it to the log entry
					char dateBuffer[64];
					std::time_t rawTime;
					struct tm *timeInfo = nullptr;

					time(&rawTime);
					timeInfo = localtime(&rawTime);
					strftime(dateBuffer, sizeof (dateBuffer), "%a, %d %b %Y %T", timeInfo);

					std::cout << dateBuffer << "->";

					// Adds the loggerlevel prefix,
					// and the thread id etcetera
					switch (this->l_Level)
					{
						case LoggerLevel::DEBUG:
						{
							std::cout << "\033[36m[debug@" << this->l_Prefix << "]: \033[0m";
							break;
						}
						case LoggerLevel::PARSER:
						{
							std::cout << "\033[34m[parser@" << this->l_Prefix << "]: \033[0m";
							break;
						}
						case LoggerLevel::INFO:
						{
							std::cout << "\033[32m[info@" << this->l_Prefix << "]: \033[0m";
							break;
						}
						case LoggerLevel::WARN:
						{
							std::cout << "\033[33m[warn@" << this->l_Prefix << "]: \033[0m";
							break;
						}
						case LoggerLevel::ERROR:
						{
							std::cout << "\033[31m[err@" << this->l_Prefix << "]: \033[0m";
							break;
						}
						case LoggerLevel::FATAL:
						{
							std::cout << "\033[31m[fatal@" << this->l_Prefix << "]: \033[0m";
							break;
						}
					}


					// Prints the message to the console
					// and clears the buffers
					if (a == LoggerOpts::FLUSH) std::cout << this->l_Stream.str() << std::flush;
					else std::cout << this->l_Stream.str() << std::endl;
					this->l_Stream.str("");
					this->l_Stream.clear();
					break;
				}
			}

			return *this;
		}

		/**
		 * Performs the operator overloading
		 *
		 * @Param {const T &} a
		 * @Return {Logger &}
		 */
		template<typename T>
		Logger &operator << (const T &a)
		{
			return this->append(a);
		}
	private:
		std::ostringstream l_Stream;
		std::string l_Prefix;
		LoggerLevel l_Level;
		LoggerLevel l_Old;
	};
}