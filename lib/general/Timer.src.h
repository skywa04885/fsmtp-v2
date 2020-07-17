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
#include <chrono>
#include <string>

#include "Logger.src.h"

namespace FSMTP
{
	class Timer
	{
	public:
		/**
		 * Initializes the timer with an specific name
		 *
		 * @Param {const std::string &} t_Prefix
		 * @Param {Logger &} t_Logger
		 * @Return {void}
		 */
		Timer(const std::string &t_Prefix, Logger &t_Logger);

		/**
		 * Default destructor which calls the timer print
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		~Timer();
	private:
		std::string t_Prefix;
		Logger &t_Logger;
		int64_t t_StartTime;
	};
}