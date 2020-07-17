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

#include "connections.src.h"

namespace FSMTP::Connections
{
	/**
	 * Default constructor for the connection
	 *
	 * @Param {const char *} hosts
	 * @Return {void}
	 */
	CassandraConnection::CassandraConnection(const char *hosts)
	{
		this->c_Cluster = cass_cluster_new();
		this->c_ConnectFuture = nullptr;
		this->c_Session = cass_session_new();

		// Sets the contact points and connects to the
		// - cassandra server, or datacenter idk
		cass_cluster_set_contact_points(this->c_Cluster, hosts);
		this->c_ConnectFuture = cass_session_connect(this->c_Session, this->c_Cluster);

		// Validates the connection and throws error of we
		// - could not connect to the cluster
		if (cass_future_error_code(this->c_ConnectFuture) != CASS_OK)
		{
			std::string message = "cass_session_connect() failed: ";
			message += CassandraConnection::getError(this->c_ConnectFuture);
			throw std::runtime_error(message);
		}
	}

	/**
	 * Default destructor which closes the connection
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	CassandraConnection::~CassandraConnection()
	{
		// Frees the cassandra used memory and probally
		// - automatically closes the session to the database
		cass_future_free(this->c_ConnectFuture);
		cass_cluster_free(this->c_Cluster);
		CassFuture *future = cass_session_close(this->c_Session);
		cass_future_wait(future);
		cass_future_free(future);
		cass_session_free(this->c_Session);
	}

	/**
	 * Creates the redis connection, so default constructor
	 *
	 * @Param {const char *} ip
	 * @Param {int32_t} port
	 * @Return {void}
	 */
	RedisConnection::RedisConnection(const char *ip, const int32_t port)
	{
		// Connects to redis and throws an error
		// - if something goes wrong
		this->r_Session = redisConnect(ip, port);

		if (this->r_Session == nullptr || this->r_Session->err)
		{
			if (this->r_Session)
			{
				// Gets the error string and throws it
				std::string message = "redisConnect() failed: ";
				message += this->r_Session->errstr;
				throw std::runtime_error(message);
			} else throw std::runtime_error("Could not allocate memory for redis");
		}
	}

	/**
	 * Default destructor, closes the connection
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	RedisConnection::~RedisConnection()
	{
		redisFree(this->r_Session);
	}

	/**
	 * Gets an cassandra error message
	 *
	 * @Param {CassFuture *} future
	 * @Return {std::string}
	 */
	std::string CassandraConnection::getError(CassFuture *future)
	{
		const char *err = nullptr;
		std::size_t errLen;

		cass_future_error_message(future, &err, &errLen);
		std::string error(err, errLen);
		
		return error;
	}
}