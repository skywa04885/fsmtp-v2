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

#include "IMAP.src.h"
#include "IMAPClientSocket.src.h"
#include "IMAPServerSession.src.h"
#include "IMAPCommand.src.h"
#include "IMAPResponse.src.h"

namespace FSMTP::IMAP::MESSAGE_HANDLER
{
	typedef enum : uint8_t
	{
		LQT_ALL = 0,
		LQT_LAYERS
	} ListQueryType;

	typedef struct
	{
		ListQueryType l_Type;
		std::size_t l_Depth;
	} listQuery;

	/**
	 * Gets the list query type from a string
	 *
	 * @Param {const std::string &} arg0
	 * @Param {const std::string &} arg1
	 * @Return {listQuery}
	 */
	listQuery listQueryFromString(const std::string &arg0, const std::string &arg1);

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
		IMAPServerSession &session,
		RedisConnection *redis,
		CassandraConnection *cassandra
	);
}