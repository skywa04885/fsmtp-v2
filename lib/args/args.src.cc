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

#include "args.src.h"

namespace FSMTP
{
	/**
	 * Parses an raw string of arguments into an argument
	 * - vector
	 * 
	 * @Param {const std::string &} raw
	 * @Return {std::vector<CMDArg>}
	 */
	std::vector<CMDArg> CMDArg::parse(const std::string &raw)
	{
		std::vector<CMDArg> res = {};

		// Loops over the arguments and parses them
		std::string token;
		std::stringstream stream(raw);
		while (std::getline(stream, token, '-'))
		{
			// Gets the value of index, and appends
			// - the current string if it is not there
			std::size_t index = token.find_first_of('=');
			if (index == std::string::npos)
				res.push_back(CMDArg(raw, ""));

			// Pushes the key with the value to the result vector
			res.push_back(CMDArg(raw.substr(0, index), raw.substr(index+1)));
		}

		// Returns the vector
		return res;
	}

	/**
	 * Default constructor for an command line argument
	 * 
	 * @Param {const std::String &} c_Name
	 * @Param {const std::vector<std::string &} c_Arg
	 */
	CMDArg::CMDArg(
	    const std::string &c_Name,
	    const std::string &c_Arg
	):
		c_Name(c_Name), c_Arg(c_Arg)
	{}

	/**
	 * The empty default constructor for an command line
	 * - argument
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	CMDArg::CMDArg(void)
	{}

	/**
	 * Handles the arguments from the FSMTP program
	 *
	 * @Param {const std::string &} argv
	 * @Return {void}
	 */
	void handleArguments(const std::string &argv)
	{
		std::vector<CMDArg> arguments = CMDArg::parse(argv);
	}
}