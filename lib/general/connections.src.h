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
#include <filesystem>
#include <hiredis/hiredis.h>

#include <cassandra.h>

namespace FSMTP::Connections
{
	class CassandraConnection
	{
	public:
		/**
		 * Gets an cassandra error message
		 *
		 * @Param {CassFuture *} future
		 * @Return {std::string}
		 */
		static std::string getError(CassFuture *future);

		/**
		 * Default constructor for the connection
		 *
		 * @Param {const char *} hosts
		 * @Return {void}
		 */
		explicit CassandraConnection(const char *hosts);

		/**
		 * Default destructor which closes the connection
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		~CassandraConnection();

		CassSession *c_Session;
		CassCluster *c_Cluster;
		CassFuture *c_ConnectFuture;
	};

	class RedisConnection
	{
	public:
		/**
		 * Creates the redis connection, so default constructor
		 *
		 * @Param {const char *} ip
		 * @Param {int32_t} port
		 * @Return {void}
		 */
		explicit RedisConnection(const char *ip, const int32_t port);

		/**
		 * Default destructor, closes the connection
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		~RedisConnection();
	
		redisContext *r_Session;
	};
}