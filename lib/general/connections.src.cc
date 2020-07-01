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
			// Gets the error message and combines it
			// - with our own message, then throws it
			const char *err = nullptr;
			std::size_t errLen;
			cass_future_error_message(this->c_ConnectFuture, &err, &errLen);

			char message[1024];
			sprintf(message, "cass_session_connect() failed: %.*s", errLen, err);
			throw std::runtime_error(message);
		}
	}

	CassandraConnection::~CassandraConnection()
	{
		// Frees the cassandra used memory and probally
		// - automatically closes the session to the database
		cass_future_free(this->c_ConnectFuture);
		cass_cluster_free(this->c_Cluster);
		cass_session_free(this->c_Session);
	}
}