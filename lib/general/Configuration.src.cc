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

#include "Configuration.src.h"

Json::Value _config;

namespace FSMTP
{
	/**
	 * Reads the configuration file
	 *
	 * @Param {const char *} filename
	 * @Return {void}
	 */
	void Configuration::read(const char *filename)
	{
		DEBUG_ONLY(Logger logger("CReader", LoggerLevel::DEBUG));

		// Creates the stream, and checks for errors
		std::ifstream stream(filename, std::ifstream::binary);
		if (!stream.is_open())
		{
			std::string error = "std::ifstream stream(\"";
			error += filename;
			error += "\") failed: ";
			error += strerror(errno);
			throw std::runtime_error(EXCEPT_DEBUG(error));
		}

		// Prints the configuration
		DEBUG_ONLY(logger << "Configuratie bestand wordt uitgelezen: " << filename << ENDL);

		// Reads the config
		std::string error;
		Json::CharReaderBuilder builder;
		if (!Json::parseFromStream(builder, stream, &_config, &error))
		{
			throw std::runtime_error(EXCEPT_DEBUG(error));
		}

		// Closes the stream
		stream.close();
	}
}