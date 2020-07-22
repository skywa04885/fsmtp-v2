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

#include "MailboxStatus.src.h"

namespace FSMTP::Models
{
	/**
	 * Empty constructor for the mailbox status
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	MailboxStatus::MailboxStatus(void):
		s_Recent(0), s_Total(0), s_Flags(0x0),
		s_PerfmaFlags(0x0), s_NextUID(0), s_Unseen(0),
		s_Bucket(0)
	{}

	/**
	 * Gets the mailbox status
	 *
	 * @Param {RedisConnection *} redis
	 * @Param {CassandraConnection *} cassandra
	 * @Param {const int64_t} s_Bucket
	 * @Param {const std::string &} s_Domain
	 * @Param {const CassUuid &} uuid
	 * @Param {const std::string &} mailboxPath
	 */
	MailboxStatus MailboxStatus::get(
		RedisConnection *redis,
		CassandraConnection *cassandra,
		const int64_t s_Bucket,
		const std::string &s_Domain,
		const CassUuid &uuid,
		const std::string &mailboxPath
	)
	{
		MailboxStatus res;

		// =================================
		// Attempts to get the status
		//
		// If this fails, we will create
		// - one based on the database
		// =================================

		// Builds the command
		std::string prefix = "mstat:";
		prefix += std::to_string(s_Bucket);
		prefix += ':';
		prefix += s_Domain;
		prefix += ':';
		prefix += std::to_string(cass_uuid_timestamp(uuid));
		prefix += ':';
		prefix += mailboxPath;

		// Builds the command
		std::string command = "HGETALL ";
		command += prefix;

		// Executes the command
		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command.c_str()
		));
		// Checks if we even received something
		if (reply->type != REDIS_REPLY_NIL && reply->elements > 0)
		{
			// Checks if the reply object is valid
			if (reply->type != REDIS_REPLY_ARRAY)
			{
				freeReplyObject(reply);
				throw DatabaseException(EXCEPT_DEBUG("Expected type array, got something else .."));
			}

			// Gets the flags
			if (reply->element[1]->type != REDIS_REPLY_STRING)
			{
				freeReplyObject(reply);
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_Flags = std::stoi(reply->element[1]->str);

			// Gets the unseen count
			if (reply->element[3]->type != REDIS_REPLY_STRING)
			{
				freeReplyObject(reply);
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_Unseen = std::stoi(reply->element[3]->str);

			// Gets the total count
			if (reply->element[5]->type != REDIS_REPLY_STRING)
			{
				freeReplyObject(reply);
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_Total = std::stoi(reply->element[5]->str);

			// Gets the perma flags
			if (reply->element[7]->type != REDIS_REPLY_STRING)
			{
				freeReplyObject(reply);
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_PerfmaFlags = std::stoi(reply->element[7]->str);

			// Gets the next UID
			if (reply->element[9]->type != REDIS_REPLY_STRING)
			{
				freeReplyObject(reply);
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_NextUID = std::stoi(reply->element[9]->str);

			// Gets the most recent one
			if (reply->element[11]->type != REDIS_REPLY_STRING)
			{
				freeReplyObject(reply);
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_Recent = std::stoi(reply->element[11]->str);

			// Returns the result
			freeReplyObject(reply);

			// ======================================
			// Updates the status
			//
			// Since we're reading it, we can reset
			// - the recent counter
			// ======================================

			// Builds the new command
			command = "HMSET ";
			command += prefix;
			command += " v6 0";

			// Executes the command and checks for errors
			reply = reinterpret_cast<redisReply *>(redisCommand(
				redis->r_Session, command.c_str()
			));
			if (reply->type == REDIS_REPLY_ERROR)
			{
				std::string error = "redisCommand() failed: ";
				error += std::string(reply->str, reply->len);
				freeReplyObject(reply);
				throw DatabaseException(EXCEPT_DEBUG(error));
			}


			// Free's the memory and returns the result
			freeReplyObject(reply);
			return res;
		} else freeReplyObject(reply);
		
		// =================================
		// Builds the status
		//
		// Since it does not exists
		// - we generate one
		// =================================

		// Restores the data from cassandra
		res = MailboxStatus::restoreFromCassandra(
			cassandra,
			s_Bucket,
			s_Domain,
			uuid,
			mailboxPath
		);

		// Saves the data in redis
		res.save(redis, mailboxPath);

		return res;
	}

	/**
	 * Saves the mailbox status
	 *
	 * @Param {RedisConnection *} redis
	 * @Return {void}
	 */
	void MailboxStatus::save(RedisConnection *redis, const std::string &mailboxPath)
	{
		// Builds the command
		std::string command = "HMSET mstat:";
		command += std::to_string(this->s_Bucket);
		command += ':';
		command += s_Domain;
		command += ':';
		command += std::to_string(cass_uuid_timestamp(this->s_UUID));
		command += ':';
		command += mailboxPath;

		// Appends the values
		command += " v1 ";
		command += std::to_string(this->s_Flags);
		command += " v2 ";
		command += std::to_string(this->s_Unseen);
		command += " v3 ";
		command += std::to_string(this->s_Total);
		command += " v4 ";
		command += std::to_string(this->s_PerfmaFlags);
		command += " v5 ";
		command += std::to_string(this->s_NextUID);
		command += " v6 ";
		command += std::to_string(this->s_Recent);

		// Executes the command
		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session,
			command.c_str()
		));
		if (reply->type == REDIS_REPLY_ERROR)
		{
			std::string error = "redisCommand() failed: ";
			error += std::string(reply->str, reply->len);
			freeReplyObject(reply);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}

		// Frees the memory and returns
		freeReplyObject(reply);
	}

	/**
	 * Restores an mailbox status from cassandra (EXPENSIVE)
	 *
	 * @Param {CassandraConnection *} cassandra
	 * @Param {const std::string &} domain
	 * @Param {const CassUuid &} uuid
	 * @Param {const std::string &} mailboxPath
	 * @Return {MailboxStatus}
	 */
	MailboxStatus MailboxStatus::restoreFromCassandra(
		CassandraConnection *cassandra,
		const int64_t bucket,
		const std::string &domain,
		const CassUuid &uuid,
		const std::string &mailboxPath
	)
	{
		MailboxStatus res;
		res.s_Bucket = bucket;
		res.s_Domain = domain;
		res.s_UUID = uuid;
		int32_t largestUID = 0;

		// Gets the mailbox itself, and sets
		// - the already known variables
		Mailbox mailbox = Mailbox::get(cassandra, bucket, domain, uuid, mailboxPath);
		res.s_Total = mailbox.e_MessageCount;
		res.s_Flags = mailbox.e_Flags;

		// Starts looping over the messages, and checking
		// - the flags
		const char *query = R"(SELECT e_flags, e_uid FROM fannst.email_shortcuts 
		WHERE e_domain=? AND e_owners_uuid=? AND e_mailbox=?)";
		CassStatement *statement = nullptr;

		statement = cass_statement_new(query, 3);
		cass_statement_bind_string(statement, 0, domain.c_str());
		cass_statement_bind_uuid(statement, 1, uuid);
		cass_statement_bind_string(statement, 2, mailboxPath.c_str());
		cass_statement_set_paging_size(statement, 50);

		cass_bool_t hasMorePages;
		do {
			CassError rc;
			CassFuture *future = nullptr;

			// ===============================
			// Executes
			//
			// Executes the query, and checks
			// - for errors
			// ===============================

			future = cass_session_execute(cassandra->c_Session, statement);
			cass_future_wait(future);

			rc = cass_future_error_code(future);
			if (rc != CASS_OK)
			{
				std::string error = "cass_session_execute() failed: ";
				error += CassandraConnection::getError(future);
				cass_future_free(future);
				cass_statement_free(statement);
				throw DatabaseException(error);
			}

			// ===============================
			// Iterates
			//
			// Loops over the results, and
			// - processes the data
			// ===============================

			const CassResult *result = cass_future_get_result(future);
			CassIterator *iterator = cass_iterator_from_result(result);

			while (cass_iterator_next(iterator))
			{
				const CassRow *row = cass_iterator_get_row(iterator);
				int32_t flags, uid;

				// Gets the values
				cass_value_get_int32(
					cass_row_get_column_by_name(row, "e_flags"),
					&flags
				);
				cass_value_get_int32(
					cass_row_get_column_by_name(row, "e_uid"),
					&uid
				);

				// Checks if it is unseen
				if (!(BINARY_COMPARE(flags, _EMAIL_FLAG_SEEN)))
				{
					++res.s_Unseen;
				}

				// Checks if this is the largest UID
				if (uid > largestUID) largestUID = uid;
			}

			// Checks if there are more pages
			// - and handles them
			hasMorePages = cass_result_has_more_pages(result);
			if (hasMorePages)
				cass_statement_set_paging_state(statement, result);

			cass_future_free(future);
			cass_result_free(result);
			cass_iterator_free(iterator);
		} while (hasMorePages);

		// Sets the next UID
		res.s_NextUID = ++largestUID;

		// Frees the statement
		cass_statement_free(statement);		
		return res;
	}

	/**
	 * Adds an new message to an mailbox
	 *
	 * @Param {RedisConnection *} redis
 	 * @Param {CassandraConnection *} cassandra
	 * @Param {const int64_t} s_Bucket
	 * @Param {const std::string &} s_Domain
	 * @Param {const CassUuid &} uuid
	 * @Param {const std::string &} mailboxPath
	 */
	static void addOneMessage(
		RedisConnection *redis,
		CassandraConnection *cassandra,
		const int64_t s_Bucket,
		const std::string &s_Domain,
		const CassUuid &uuid,
		const std::string &mailboxPath
	)
	{
		int32_t prevUid, prevTotal;

		// ==================================
		// Gets the original value
		//
		// If this fails, create new status
		// - and then continue
		// ==================================
	}
}