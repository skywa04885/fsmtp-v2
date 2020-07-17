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
#include <vector>
#include <sstream>

namespace FSMTP
{
    class CMDArg
    {
    public:
        /**
         * Parses an raw string of arguments into an argument
         * - vector
         * 
         * @Param {const std::string &} raw
         * @Return {std::vector<CMDArg>}
         */
        static std::vector<CMDArg> parse(const std::string &raw);

        /**
         * Default constructor for an command line argument
         * 
         * @Param {const std::String &} c_Name
         * @Param {const std::vector<std::string &} c_Arg
         */
        CMDArg(
            const std::string &c_Name,
            const std::string &c_Arg
        );

        /**
         * The empty default constructor for an command line
         * - argument
         *
         * @Param {void}
         * @Return {void}
         */
        explicit CMDArg(void);
    private:
        std::string c_Arg;
        std::string c_Name;
    };

    /**
     * Handles the arguments from the FSMTP program
     *
     * @Param {const std::string &} argv
     * @Return {void}
     */
    void handleArguments(const std::string &argv);
}