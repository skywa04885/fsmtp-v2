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

#include <tuple>
#include <string>
#include <iostream>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../../general/connections.src.h"
#include "../../general/Passwords.src.h"
#include "../../models/LocalDomain.src.h"
#include "../../models/Account.src.h"
#include "../../models/Email.src.h"

using namespace FSMTP::Connections;
using namespace FSMTP::Models;

namespace FSMTP::Server
{
	/**
	 * Parses the username and password from the base64 hash
	 *
	 * @Param {const std::string &} hash
	 * @Return {std::string}
	 * @Return {std::string}
	 */
	std::tuple<std::string, std::string> getUserAndPassB64(
		const std::string &hash
	);

	/**
	 * Verifies an authentication entry
	 *
	 * @Param {RedisConnection *} redis
	 * @Param {CassandraConnection *} cassandra
	 * @Param {const std::string &} user
	 * @Param {const std::string &} password
	 * @Param {AccountShortut &} shortcutTarget
	 * @Return {bool}
	 */
	bool authVerify(
		RedisConnection *redis,
		CassandraConnection *cassandra,
		const std::string &user,
		const std::string &password,
		AccountShortcut &shortcutTarget
	);
}