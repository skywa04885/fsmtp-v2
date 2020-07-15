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

#include "LocalDomain.src.h"

namespace FSMTP::Models
{
	/**
	 * Creates an new local domain and generates
	 * the uuid automatically for it
	 * 
	 * @Param {std::string} l_Domain
	 * @Return void
	 */
	LocalDomain::LocalDomain(const std::string &l_Domain):
		l_Domain(l_Domain)
	{
		// Creates the UUID gen and generates an new 
		// - user id for the domain
		CassUuidGen *uuidGen = cass_uuid_gen_new();
		cass_uuid_gen_time(uuidGen, &this->l_UUID);
		cass_uuid_gen_free(uuidGen);
	}

	/**
	 * Empty constructor, will just initialize the values

	 * @Param void
	 * @Return void
	 */
	LocalDomain::LocalDomain(void):
		l_Domain()
	{}

	/**
	 * Searches in the database for an domain with that
	 * specific ID
	 *
	 * @Param {const std::string &} l_Domain
	 * @Param {std::unique_ptr<CassandraConnection> &} database
	 * @Return void
	 */
	LocalDomain LocalDomain::getByDomain(
		const std::string &l_Domain,
		std::unique_ptr<CassandraConnection>& database
	)
	{
		LocalDomain res;
		CassStatement *queryStatement = nullptr;
		CassFuture *queryFuture = nullptr;
		CassError rc;

		// Creates the query and binds the parameters,
		// - after this we execute the query and wait
		// - for results
		const char *query = R"(SELECT e_domain_uuid FROM fannst.local_domain WHERE e_domain = ?)";
		
		queryStatement = cass_statement_new(query, 1);
		cass_statement_bind_string(queryStatement, 0, l_Domain.c_str());

		queryFuture = cass_session_execute(database->c_Session, queryStatement);
		cass_future_wait(queryFuture);

		// Checks if the query was successfully
		// - if not throws error
		rc = cass_future_error_code(queryFuture);
		if (rc != CASS_OK)
		{
			std::string message = "cass_session_execute() failed: ";
			message += message += CassandraConnection::getError(queryFuture);

			// Frees the memory
			cass_future_free(queryFuture);
			cass_statement_free(queryStatement);
			
			// Throws the exception
			throw DatabaseException(message);
		}

		// Gets the data from the query, and stores it in
		// - the current instance
		const CassResult *result = cass_future_get_result(queryFuture);
		const CassRow *row = cass_result_first_row(result);

		if (!row)
		{
			// Frees the memory
			cass_future_free(queryFuture);
			cass_result_free(result);
			cass_statement_free(queryStatement);

			// Trows the empty query error
			throw EmptyQuery("No LocalDomain found with this specific domain");
		}

		cass_value_get_uuid(cass_row_get_column_by_name(row, "u_uuid"), &res.l_UUID);

		// Frees the memory since the query was already
		// - performed, and we don't want to leak anything
		cass_statement_free(queryStatement);
		cass_future_free(queryFuture);
		cass_result_free(result);

		return res;
	}

	/**
	 * Finds an domain and its uuid in the redis db
	 *
	 * @Param {const std::string &} l_Domain
	 * @Param {RedisConnection *} redis
	 * @Return {LocalDomain}
	 */
	LocalDomain LocalDomain::findRedis(
		const std::string &l_Domain,
		RedisConnection *redis
	)
	{
		LocalDomain res;

		// Prepares the command
		std::string command = "GET domain:";
		command += l_Domain;

		// Performs the command and gets the result
		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command.c_str()
		));
		if (reply->type == REDIS_REPLY_NIL)
		{
			freeReplyObject(reply);
			throw EmptyQuery("Could not find domain: " + l_Domain);
		} else if (reply->type != REDIS_REPLY_STRING)
		{
			freeReplyObject(reply);
			throw DatabaseException("Data type string expected, found different !");
		}

		// Sets the data in the result
		res.l_Domain = l_Domain;
		cass_uuid_from_string(reply->str, &res.l_UUID);

		// Frees the memory, and returns the result
		freeReplyObject(reply);
		return res;
	}

	/**
	 * Gets all the domains from the cassandra database
	 *
	 * @Param {CassandraConnection *} cass
	 * @Return {std::vector<LocalDomain>}
	 */
	std::vector<LocalDomain> LocalDomain::findAllCassandra(
		CassandraConnection *cass
	)
	{
		std::vector<LocalDomain> res = {};
		CassStatement *statement = nullptr;
		CassFuture *future = nullptr;
		CassError rc;
		const char *query = "SELECT * FROM fannst.local_domain";

		// Creates the statement and performs the
		// - query, and checks for errors
		statement = cass_statement_new(query, 0);
		future = cass_session_execute(cass->c_Session, statement);
		cass_future_wait(future);

		rc = cass_future_error_code(future);
		if (rc != CASS_OK)
		{
			std::string message = "cass_session_execute() failed: ";
			message += CassandraConnection::getError(future);

			cass_future_free(future);
			cass_statement_free(statement);
			throw DatabaseException(message);
		}

		// Starts looping over the data and builds
		// - the result
		const CassResult *result = nullptr;
		CassIterator *iterator = nullptr;

		result = cass_future_get_result(future);
		iterator = cass_iterator_from_result(result);

		while (cass_iterator_next(iterator))
		{
			LocalDomain temp;

			std::size_t domainBufferLen;
			const char *domainBuffer = nullptr;
			const CassRow *row = cass_iterator_get_row(iterator);

			cass_value_get_uuid(
				cass_row_get_column_by_name(row, "e_domain_uuid"), 
				&temp.l_UUID
			);
			cass_value_get_string(
				cass_row_get_column_by_name(row, "e_domain"),
				&domainBuffer,
				&domainBufferLen
			);

			temp.l_Domain = std::string(domainBuffer, domainBufferLen);
			res.push_back(temp);
		}

		// Frees the memory and returns
		cass_future_free(future);
		cass_result_free(result);
		cass_iterator_free(iterator);
		cass_statement_free(statement);
		return res;
	}
}