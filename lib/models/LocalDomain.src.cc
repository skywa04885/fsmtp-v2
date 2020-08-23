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
	LocalDomain::LocalDomain(const string &l_Domain):
		l_Domain(l_Domain)
	{
		CassUuidGen *uuidGen = cass_uuid_gen_new();
		cass_uuid_gen_time(uuidGen, &this->l_UUID);
		cass_uuid_gen_free(uuidGen);
	}

	LocalDomain::LocalDomain():
		l_Domain()
	{}

	void LocalDomain::saveRedis(RedisConnection *redis) {
		char command[256], prefix[128], uuid[CASS_UUID_STRING_LENGTH];

		LocalDomain::getPrefix(this->l_Domain, prefix);
		cass_uuid_string(this->l_UUID, uuid);
		sprintf(command, "SET %s %s", prefix, uuid);

		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command
		));
		DEFER(freeReplyObject(reply));

		if (reply->type == REDIS_REPLY_ERROR) {
			throw DatabaseException("Could not store local domain");
		}
	}

	void LocalDomain::getPrefix(const string &l_Domain, char *buffer) {
		sprintf(buffer, "domain:%s", l_Domain.c_str());	
	}

	LocalDomain LocalDomain::getByDomain(
		const string &l_Domain,
		CassandraConnection *database
	) {
		const char *query = R"(SELECT e_domain_uuid FROM fannst.local_domain WHERE e_domain = ?)";
		LocalDomain res;
		CassStatement *statement = nullptr;
		CassFuture *future = nullptr;

		// Creates the statement and performs the query, and if something
		//  goes wrong we throw the cassandra error, with the according message

		statement = cass_statement_new(query, 1);
		DEFER(cass_statement_free(statement));
		cass_statement_bind_string(statement, 0, l_Domain.c_str());

		future = cass_session_execute(database->c_Session, statement);
		DEFER(cass_future_free(future));
		cass_future_wait(future);

		if (cass_future_error_code(future) != CASS_OK) {
			string message = "cass_session_execute() failed: ";
			message += message += CassandraConnection::getError(future);
			throw DatabaseException(message);
		}

		// Gets the query results, and throws an error if it could not be found
		//  if the result is there, we get the uuid and store it in the result instance

		const CassResult *result = cass_future_get_result(future);
		const CassRow *row = cass_result_first_row(result);
		DEFER(cass_result_free(result));

		if (!row) {
			throw EmptyQuery("No LocalDomain found with this specific domain");
		}

		cass_value_get_uuid(cass_row_get_column_by_name(row, "u_uuid"), &res.l_UUID);
		res.l_Domain = l_Domain;

		return res;
	}

	LocalDomain LocalDomain::findRedis(const string &l_Domain, RedisConnection *redis) {
		LocalDomain res;

		char command[256], prefix[128];
		LocalDomain::getPrefix(l_Domain, prefix);
		sprintf(command, "GET %s", prefix);

		// Performs the redis get command, after which we will get the
		//  uuid of the domain, this does not matter but will check
		//  if an domain is local or not, after this we store the instance
		//  variables.

		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command
		));
		DEFER(freeReplyObject(reply));

		if (reply->type == REDIS_REPLY_NIL) {
			throw EmptyQuery("Could not find domain: " + l_Domain);
		} else if (reply->type != REDIS_REPLY_STRING) {
			throw DatabaseException("Data type string expected, found different !");
		}

		res.l_Domain = l_Domain;
		cass_uuid_from_string(reply->str, &res.l_UUID);

		return res;
	}

	vector<LocalDomain> LocalDomain::findAllCassandra(CassandraConnection *cass) {
		const char *query = "SELECT * FROM fannst.local_domain";
		vector<LocalDomain> res = {};
		CassStatement *statement = nullptr;
		CassFuture *future = nullptr;

		// Performs the query and throws an error of the execution failed
		//  after this we will query the results

		statement = cass_statement_new(query, 0);
		future = cass_session_execute(cass->c_Session, statement);
		cass_future_wait(future);
		DEFER_M({
			cass_statement_free(statement);
			cass_future_free(future);
		});

		if (cass_future_error_code(future) != CASS_OK) {
			string message = "cass_session_execute() failed: ";
			message += CassandraConnection::getError(future);
			throw DatabaseException(message);
		}

		// Gets the query result, and starts looping over the domains
		//  while storing the domains inside of the result vector

		const CassResult *result = nullptr;
		CassIterator *iterator = nullptr;

		result = cass_future_get_result(future);
		iterator = cass_iterator_from_result(result);
		DEFER_M({
			cass_result_free(result);
			cass_iterator_free(iterator);
		});

		while (cass_iterator_next(iterator)) {
			LocalDomain temp;

			size_t domainBufferLen;
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

			temp.l_Domain = string(domainBuffer, domainBufferLen);
			res.push_back(temp);
		}

		return res;
	}

	 LocalDomain LocalDomain::get(
		const string &l_Domain,
		CassandraConnection *cass,
		RedisConnection *redis
	) {
		// Attempts an redis query for an local domain, if it does not exists
		//  check cassandra, if it not exists again throw error, else we will
		//  store the local domain from cassandra inside redis.

		try {
			return LocalDomain::findRedis(l_Domain, redis);
		} catch (const EmptyQuery &e) {
			LocalDomain domain = LocalDomain::getByDomain(l_Domain, cass);
			domain.saveRedis(redis);
			return domain;
		}
	}
}
