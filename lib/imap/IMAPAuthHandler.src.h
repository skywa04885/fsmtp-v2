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
#include "IMAPCommand.src.h"
#include "IMAPServerSession.src.h"

namespace FSMTP::IMAP::AUTH_HANDLER
{
	/**
	 * Handles the 'LOGIN' command
	 *
	 * @Param {IMAPClientSocket *} client
	 * @Param {IMAPCommand &} command
	 * @Param {IMAPServerSession &} session
	 * @Param {RedisConnection *} redis
	 * @Param {CassandraConnection *} cassandra
	 * @Return {void}
 	 */
	void login(
		IMAPClientSocket *client,
		IMAPCommand &command,
		IMAPServerSession &sessios,
		RedisConnection *redis,
		CassandraConnection *cassandra
	);
}