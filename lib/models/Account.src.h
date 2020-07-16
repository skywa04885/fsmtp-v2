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
#include <chrono>
#include <cstring>
#include <memory>
#include <iostream>

#include <cassandra.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <hiredis/hiredis.h>

#include "../general/connections.src.h"
#include "../general/exceptions.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
	class Account
	{
	public:
		/**
		 * Default empty constructor for the account
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit Account(void);

		/**
		 * Saves an account in the cassandra database
		 *
		 * @Param {CassadraConnection *} cassandra
		 * @Return {void}
		 */
		void save(CassandraConnection *cassandra);

		/**
		 * Generates an RSA keypair for the account
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void generateKeypair(void);

		/**
		 * Gets the current user bucket
		 *
		 * @Param {void}
		 * @Return {int64_t}
		 */
		static int64_t getBucket(void);

		/**
		 * Gets the password and public key
		 *
		 * @Param {CassandraConnection *} client
		 * @Param {const std::string &} domain
		 * @Param {const int64_t} bucket
		 * @Param {const CassUuid &} uuid
		 * @Return {std::string}
		 * @Return {std::string}
		 */
		static std::tuple<std::string, std::string> getPassAndPublicKey(
			CassandraConnection *client,
			const std::string &domain,
			const int64_t bucket,
			const CassUuid &uuid
		);

		std::string a_Username;
		std::string a_PictureURI;
		std::string a_Password;
		std::string a_Domain;
		int64_t a_Bucket;
		std::string a_FullName;
		int64_t a_BirthDate;
		int64_t a_CreationDate;
		std::string a_RSAPublic;
		std::string a_RSAPrivate;
		double a_Gas;
		std::string a_Country;
		std::string a_Region;
		std::string a_City;
		std::string a_Address;
		std::string a_Phone;
		int8_t a_Type;
		CassUuid a_UUID;
		int64_t a_Flags;
		int64_t a_StorageUsedInBytes;
		int64_t a_StorageMaxInBytes;
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
		 * Stores an account shortcut in cassandra
		 *
		 * @Param {CassandraConnection *} conn
		 * @Return {void}
		 */
		void save(CassandraConnection *conn);

		/**
		 * Finds an account shortcut in the cassandra
		 * - database
		 *
		 * @Param {std::unique_ptr<CassandraConnection> *} conn
		 * @Param {const std::string &} domain
		 * @Param {const std::string &} username
		 * @Return {AccountShortuct}
		 */
		static AccountShortcut find(
			CassandraConnection *conn,
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

		/**
		 * Stores the shortcut in redis
		 *
		 * @Param {RedisConnection *} redis
		 * @Return {void}
		 */
		void saveRedis(RedisConnection *redis);

		/**
		 * Equals operator overload
		 *
		 * @Param {const AccountShortcut &} a
		 * @Return {bool}
		 */
		bool operator==(const AccountShortcut &a)
		{
			if (a.a_Username == this->a_Username && a.a_Domain == this->a_Domain)
				return true;
			else
				return false;
		}

		/**
		 * Equals operator overload
		 *
		 * @Param {const AccountShortcut &} a
		 * @Return {bool}
		 */
		bool operator!=(const AccountShortcut &a)
		{
			if (a.a_Username != this->a_Username || a.a_Domain != this->a_Domain)
				return true;
			else
				return false;
		}

		int64_t a_Bucket;
		std::string a_Domain;
		std::string a_Username;
		CassUuid a_UUID;
	};
}