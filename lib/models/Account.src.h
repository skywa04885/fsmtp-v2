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

#include "../default.h"

#include "../general/connections.src.h"
#include "../general/exceptions.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
	class Account
	{
	public:
		explicit Account();

		void save(CassandraConnection *cassandra);
		void generateKeypair();
		static int64_t getBucket();
		static tuple<string, string> getPassAndPublicKey(
			CassandraConnection *client,
			const string &domain,
			const int64_t bucket,
			const CassUuid &uuid
		);

		string a_Username;
		string a_PictureURI;
		string a_Password;
		string a_Domain;
		int64_t a_Bucket;
		string a_FullName;
		int64_t a_BirthDate;
		int64_t a_CreationDate;
		string a_RSAPublic;
		string a_RSAPrivate;
		double a_Gas;
		string a_Country;
		string a_Region;
		string a_City;
		string a_Address;
		string a_Phone;
		int8_t a_Type;
		CassUuid a_UUID;
		int64_t a_Flags;
		int64_t a_StorageUsedInBytes;
		int64_t a_StorageMaxInBytes;
	};

	class AccountShortcut
	{
	public:
		explicit AccountShortcut();
		AccountShortcut(
			int64_t a_Bucket, const string &a_Domain,
			const string &a_Username, const CassUuid &a_UUID
		);

		void save(CassandraConnection *conn);
		void saveRedis(RedisConnection *redis);

		static void getPrefix(const string &domain, const string &username, char *buffer);
		static AccountShortcut find(
			CassandraConnection *conn, const string &domain,
			const string &username
		);
		static AccountShortcut findRedis(
			RedisConnection *redis, const string &domain,
			const string &username
		);

		bool operator == (const AccountShortcut &a) {
			if (a.a_Username == this->a_Username && a.a_Domain == this->a_Domain) {
				return true;
			} else {
				return false;
			}
		}

		bool operator!=(const AccountShortcut &a) {
			if (a.a_Username != this->a_Username || a.a_Domain != this->a_Domain) {
				return true;
			} else {
				return false;
			}
		}

		int64_t a_Bucket;
		string a_Domain;
		string a_Username;
		CassUuid a_UUID;
	};
}
