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

#include "IMAPMessageHander.src.h"

namespace FSMTP::IMAP::MESSAGE_HANDLER
{
	/**
	 * Gets the list query type from a string
	 *
	 * @Param {const std::string &} arg0
	 * @Param {const std::string &} arg1
	 * @Return {listQuery}
	 */
	listQuery listQueryFromString(const std::string &arg0, const std::string &arg1)
	{
		// Checks the type, and if we need to go more in depth
		if (arg1[0] == '*')
			return listQuery{ListQueryType::LQT_ALL, 0};
		else if (arg1[0] == '%')
		{
			std::size_t depth = 0;
			std::stringstream stream(arg1);
			std::string token;
			while (std::getline(stream, token, '/'))
				++depth;
			return listQuery{ListQueryType::LQT_LAYERS, depth};
		} else throw std::runtime_error(EXCEPT_DEBUG("Not implemented query"));
	}

	/**
	 * Handles the 'LIST' command
	 *
	 * @Param {IMAPClientSocket *} client
	 * @Param {IMAPCommand &} command
	 * @Param {IMAPServerSession &} session
	 * @Param {RedisConnection *} redis
	 * @Param {CassandraConnection *} cassandra
	 * @Return {void}
 	 */
	void list(
		IMAPClientSocket *client,
		IMAPCommand &command,
		IMAPServerSession &sessios,
		RedisConnection *redis,
		CassandraConnection *cassandra
	)
	{
		// Checks if there are enough arguments
		if (command.c_Args.size() < 2)
			throw IMAPBad("Arguments invalid");

		// Parses the arguments, and removes the quotes
		std::string &arg0 = command.c_Args[0];
		std::string &arg1 = command.c_Args[1];
		removeStringQuotes(arg0);
		removeStringQuotes(arg1);

		// Checks what the client tries to receive, and
		// - prints it to the debug console
		listQuery query = listQueryFromString(arg0, arg1);

		// Performs switch on the actions
		switch (query.l_Type)
		{
			case ListQueryType::LQT_ALL:
			{

			}
			default: throw std::runtime_error(EXCEPT_DEBUG("Query type not implemented"));
		}
	}
}