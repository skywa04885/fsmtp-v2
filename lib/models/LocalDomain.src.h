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
	class LocalDomain
	{
	public:
		LocalDomain(const std::string &l_Domain);
		LocalDomain();

		void saveRedis(RedisConnection *redis);

		static void getPrefix(const string &l_Domain, char *buffer);

		static LocalDomain getByDomain(const string &l_Domain, CassandraConnection *database);
		static LocalDomain findRedis(const string &l_Domain, RedisConnection *redis);
		static vector<LocalDomain> findAllCassandra(CassandraConnection *cass);

		static LocalDomain get(
			const string &l_Domain, CassandraConnection *cass,
			RedisConnection *redis
		);

		string l_Domain;
		CassUuid l_UUID;
	};
}