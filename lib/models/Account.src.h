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

#include <cstdint>
#include <string>
#include <cstring>
#include <memory>
#include <iostream>

#include <cassandra.h>
#include <hiredis/hiredis.h>

#include "../general/connections.src.h"
#include "../general/exceptions.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
	class Account
	{
	public:
	private:
		std::string a_Domain;
		std::string a_Username;
		std::string a_FullName;
		std::string a_Password;
		int64_t a_Bucket;
	};

	class AccountShortcut
	{
	public:
		/**
		 * Default constructor for the AccountShortcut
		 *
		 * @Param {int64_t} a_Bucket
		 * @Param {std::string &} a_Domain
		 * @Param {std::strin &} a_Username
		 * @Param {const CassUuid &} a_Uuid 
		 * @Return {void}
		 */
		AccountShortcut(
			int64_t a_Bucket,
			const std::string &a_Domain,
			const std::string &a_Username,
			const CassUuid &a_UUID
		);

		/**
		 * Empty constructor for the account shortcut
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit AccountShortcut();

		/**
		 * Finds an account shortcut in the cassandra
		 * - database
		 *
		 * @Param {std::unique_ptr<CassandraConnection> &} conn
		 * @Param {const std::string &} domain
		 * @Param {const std::string &} username
		 * @Return {AccountShortuct}
		 */
		static AccountShortcut find(
			std::unique_ptr<CassandraConnection> &conn,
			const std::string &domain,
			const std::string &username
		);

		/**
		 * Finds an user in the redis server
		 *
		 * @Param {RedisConnection *} redis
		 * @Param {std::string &} domain
		 * @Param {std::string &} username
		 * @Return {AccountShortcut}
		 */
		static AccountShortcut findRedis(
			RedisConnection *redis,
			const std::string &domain,
			const std::string &username
		);

		void save(std::unique_ptr<CassandraConnection> &conn);

		int64_t a_Bucket;
		std::string a_Domain;
		std::string a_Username;
		CassUuid a_UUID;
	};
}