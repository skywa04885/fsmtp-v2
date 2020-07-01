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
#include <cstdint>

namespace FSMTP
{
	typedef enum : uint8_t
	{
		DEBUG = 0,
		INFO,
		WARN,
		ERROR,
		FATAL
	} LoggerLevel;

	typedef enum : uint8_t
	{
		ENDL = 0,
		CLASSIC
	} LoggerOpts;

	class Logger
	{
	public:
		Logger(const std::string &l_Prefix, const LoggerLevel &l_Level):
			l_Prefix(l_Prefix), l_Level(l_Level)
		{}

		template<typename T>
		Logger &append(const T &a)
		{
			this->l_Stream << a;
			return *this;
		}

		Logger &append(const LoggerLevel& a)
		{
			this->l_Old = this->l_Level;
			this->l_Level = a;
		}

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
				{
					std::cout << 'T' << std::this_thread::get_id() << "->";

					// Adds the loggerlevel prefix,
					// and the thread id etcetera
					switch (this->l_Level)
					{
						case LoggerLevel::DEBUG:
						{
							std::cout << "\033[36m[debug@" << this->l_Prefix << "]: \033[0m";
							break;
						}
						case LoggerLevel::INFO:
						{
							std::cout << "\033[32m[informatie@" << this->l_Prefix << "]: \033[0m";
							break;
						}
						case LoggerLevel::WARN:
						{
							std::cout << "\033[33m[waarschuwing@" << this->l_Prefix << "]: \033[0m";
							break;
						}
						case LoggerLevel::ERROR:
						{
							std::cout << "\033[31m[fout@" << this->l_Prefix << "]: \033[0m";
							break;
						}
						case LoggerLevel::FATAL:
						{
							std::cout << "\033[31m[fatale fout@" << this->l_Prefix << "]: \033[0m";
							break;
						}
					}


					// Prints the message to the console
					// and clears the buffers
					std::cout << this->l_Stream.str() << std::endl;
					this->l_Stream.str("");
					this->l_Stream.clear();
					break;
				}
			}
		}

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