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

#include <string>
#include <fstream>

#include <json/json.h>
#include <string.h>

#include "macros.src.h"
#include "Logger.src.h"

namespace FSMTP
{
	class Configuration
	{
	public:
		/**
		 * Reads the configuration file
		 *
		 * @Param {const char *} filename
		 * @Return {void}
		 */
		static void read(const char *filename);
	};
};