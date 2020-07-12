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

#include "Timer.src.h"

namespace FSMTP
{
	/**
	 * Initializes the timer with an specific name
	 *
	 * @Param {const std::string &} t_Prefix
	 * @Param {Logger &} t_Logger
	 * @Return {void}
	 */
	Timer::Timer(const std::string &t_Prefix, Logger &t_Logger):
		t_Logger(t_Logger), t_Prefix(t_Prefix)
	{
		this->t_StartTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();

		this->t_Logger << DEBUG << "\033[35mTimer@" << this->t_Prefix << " started !\033[0m" << ENDL << CLASSIC;
	}

	/**
	 * Default destructor which calls the timer print
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	Timer::~Timer()
	{
		std::size_t end = std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();

		this->t_Logger << DEBUG << "\033[35mTimer@" << this->t_Prefix 
			<< " ended in [" << ((static_cast<double>(end) - static_cast<double>(this->t_StartTime)) / 1000000) 
			<< "ms, " << (end - this->t_StartTime) << "us]\033[0m" << ENDL << CLASSIC;
	}
}
