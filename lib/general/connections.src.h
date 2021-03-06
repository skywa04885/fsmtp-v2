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
#include <stdexcept>
#include <fstream>
#include <hiredis/hiredis.h>

#include <cassandra.h>

namespace FSMTP::Connections
{
	class CassandraConnection
	{
	public:
		static std::string getError(CassFuture *future);

		CassandraConnection(const char *hosts, const char *username, const char *password);
		~CassandraConnection();

		CassSession *c_Session;
		CassCluster *c_Cluster;
		CassFuture *c_ConnectFuture;
	};

	class RedisConnection
	{
	public:
		RedisConnection(const char *ip, const int32_t port);
		~RedisConnection();
	
		redisContext *r_Session;
	};
}