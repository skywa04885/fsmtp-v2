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

#include "Account.src.h"

namespace FSMTP::Models
{
	AccountShortcut::AccountShortcut(
		int64_t a_Bucket,
		std::string a_Domain,
		std::string a_Username,
		CassUuid a_UUID
	):
		a_Bucket(a_Bucket),
		a_Domain(a_Domain),
		a_Username(a_Username),
		a_UUID(a_UUID)
	{}

	AccountShortcut::AccountShortcut()
	{}

	void AccountShortcut::find(
		std::unique_ptr<CassandraConnection> &conn,
		AccountShortcut &shortcut,
		const std::string &domain,
		const std::string &username
	)
	{
		CassError rc;
		CassStatement *statement = nullptr;
		CassFuture *future = nullptr;

		// ========================================
  	// Creates the query and executes it
  	//
  	// Creates the statement and adds the final
  	// - variables, then we execute
  	// ========================================

		// Creates the query
		const char *query = R"(SELECT a_bucket, a_uuid 
		FROM fannst.account_shortcuts WHERE a_domain=? AND a_username=?)";

		// Prepares the statement and binds the
		// - values, such as domain and username
		statement = cass_statement_new(query, 2);
		cass_statement_bind_string(statement, 0, domain.c_str());
		cass_statement_bind_string(statement, 1, username.c_str());

		// Executes the query, and waits
		// - for the query to finish
		future = cass_session_execute(conn->c_Session, statement);
		cass_future_wait(future);

		// Checks if the query was successfull
		rc = cass_future_error_code(future);
		if (rc != CASS_OK)
		{
			std::string message = "cass_session_execute() failed: ";
			message += CassandraConnection::getError(future);

			throw DatabaseException(message);
		}

		// Checks if there is any data, and
		// - if so get it
		const CassResult *result = cass_future_get_result(future);
		const CassRow *row = cass_result_first_row(result);

		if (!row)
		{
			cass_result_free(result);
			cass_future_free(future);
			cass_statement_free(statement);
			
			throw EmptyQuery("Could not find AccountShortcut in database");
		}

		cass_value_get_int64(
			cass_row_get_column_by_name(row, "a_bucket"),
			&shortcut.a_Bucket
		);
		cass_value_get_uuid(
			cass_row_get_column_by_name(row, "a_uuid"),
			&shortcut.a_UUID
		);
		shortcut.a_Domain = domain;

		// Frees the memory
		cass_result_free(result);
		cass_future_free(future);
		cass_statement_free(statement);
	}

	/**
	 * Finds an user in the redis server
	 *
	 * @Param {RedisConnection *} redis
	 * @Param {std::string &} domain
	 * @Param {std::string &} username
	 * @Return {AccountShortcut}
	 */
	AccountShortcut AccountShortcut::findRedis(
		RedisConnection *redis,
		const std::string &domain,
		const std::string &username
	)
	{
		AccountShortcut res;

		// Prepares the command
		std::string command = "HGETALL acc:";
		command += username;
		command += '@';
		command += domain;

		// Performs the command
		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command.c_str()
		));

		if (reply->type == REDIS_REPLY_NIL || reply->elements <= 0)
		{
			freeReplyObject(reply);
			throw EmptyQuery("Could not find user in Redis");
		} else if (reply->type != REDIS_REPLY_ARRAY)
		{
			freeReplyObject(reply);
			throw DatabaseException("Expected type map, got something else");
		}

		if (reply->element[1]->type != REDIS_REPLY_STRING)
		{
			freeReplyObject(reply);
			throw DatabaseException("Invalid value at position 1, expected number");
		} else res.a_Bucket = std::stoi(reply->element[1]->str);

		if (reply->element[3]->type != REDIS_REPLY_STRING)
		{
			freeReplyObject(reply);
			throw DatabaseException("Invalid value at position 3: expected string");
		} cass_uuid_from_string(reply->element[3]->str, &res.a_UUID);

		freeReplyObject(reply);
		return res;
	}
}