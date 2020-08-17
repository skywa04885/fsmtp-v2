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
	static void getPrefix(
		const int64_t bucket,
		const char *domain,
		const CassUuid &uuid,
		const char *path,
		char *ret
	) {
		sprintf(
			ret, "%ld:%s:%ld:%s",
			bucket, domain,
			cass_uuid_timestamp(uuid),
			path
		);
	}

	MailboxStatus::MailboxStatus(void):
		s_Recent(0), s_Total(0), s_Flags(0x0),
		s_PerfmaFlags(0x0), s_NextUID(0), s_Unseen(0),
		s_Bucket(0)
	{}

	void MailboxStatus::clearRecent(
		RedisConnection *redis, const int64_t s_Bucket,
		const string &s_Domain, const CassUuid &uuid,
		const string &mailboxPath
	) {
		char prefix[512], command[1024];
		getPrefix(s_Bucket, s_Domain.c_str(), uuid, mailboxPath.c_str(), prefix);
		sprintf(command, "%s %s v6 %d", "HMSET", prefix, 0);

		// Performs the redis command, and checks for errors, if there is an error
		//  we throw it

		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command
		));
		DEFER(freeReplyObject(reply));

		if (reply->type == REDIS_REPLY_ERROR) {
			string error = "redisCommand() failed: ";
			error += string(reply->str, reply->len);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}
	}

	MailboxStatus MailboxStatus::get(
		RedisConnection *redis,
		CassandraConnection *cassandra,
		const int64_t s_Bucket,
		const string &s_Domain,
		const CassUuid &uuid,
		const string &mailboxPath
	) {
		MailboxStatus res;

		char prefix[512], command[1024];
		getPrefix(s_Bucket, s_Domain.c_str(), uuid, mailboxPath.c_str(), prefix);
		sprintf(command, "%s %s", "HGETALL", prefix);

		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command
		));
		DEFER(freeReplyObject(reply));

		// Gets the variables from the status stored in redis, if this is not the case
		//  we will attemp to get it from cassandra, this is more expensive thoigh

		if (reply->type != REDIS_REPLY_NIL && reply->elements > 0) {
			// Checks if the reply object is valid
			if (reply->type != REDIS_REPLY_ARRAY) {
				throw DatabaseException(EXCEPT_DEBUG("Expected type array, got something else .."));
			}

			// Gets the flags
			if (reply->element[1]->type != REDIS_REPLY_STRING) {
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_Flags = stoi(reply->element[1]->str);

			// Gets the unseen count
			if (reply->element[3]->type != REDIS_REPLY_STRING) {
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_Unseen = stoi(reply->element[3]->str);

			// Gets the total count
			if (reply->element[5]->type != REDIS_REPLY_STRING) {
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_Total = stoi(reply->element[5]->str);

			// Gets the perma flags
			if (reply->element[7]->type != REDIS_REPLY_STRING) {
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_PerfmaFlags = stoi(reply->element[7]->str);

			// Gets the next UID
			if (reply->element[9]->type != REDIS_REPLY_STRING) {
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_NextUID = stoi(reply->element[9]->str);

			// Gets the most recent one
			if (reply->element[11]->type != REDIS_REPLY_STRING) {
				throw DatabaseException(EXCEPT_DEBUG("Expected type integer, got something else .."));
			} res.s_Recent = stoi(reply->element[11]->str);

			// Updates the status and sets the recent number, since it now is received
			//  we want to set it to zero

			MailboxStatus::clearRecent(redis, s_Bucket, s_Domain, uuid, mailboxPath);

			return res;
		};

		// Since the previous status could not be found, we attempt to restore it from cassandra
		//  with the existing messages, this is expensive though

		res = MailboxStatus::restoreFromCassandra(
			cassandra,
			s_Bucket,
			s_Domain,
			uuid,
			mailboxPath
		);

		res.save(redis, mailboxPath);

		return res;
	}

	void MailboxStatus::save(RedisConnection *redis, const string &mailboxPath) {
		char prefix[512], command[1024];
		getPrefix(s_Bucket, s_Domain.c_str(), this->s_UUID, mailboxPath.c_str(), prefix); 
		sprintf(
			command, "%s %s v1 %d v2 %d v3 %d v4 %d v5 %d v6 %d",
			"HMSET", prefix, this->s_Flags, this->s_Unseen,
			this->s_Total, this->s_PerfmaFlags, this->s_NextUID,
			this->s_Recent
		);

		// Executes the command and checks for errors, if something goes
		//  wrong we throw an database exception

		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session,
			command
		));
		DEFER(freeReplyObject(reply));

		if (reply->type == REDIS_REPLY_ERROR) {
			string error = "redisCommand() failed: ";
			error += string(reply->str, reply->len);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}
	}

	MailboxStatus MailboxStatus::restoreFromCassandra(
		CassandraConnection *cassandra,
		const int64_t bucket,
		const string &domain,
		const CassUuid &uuid,
		const string &mailboxPath
	) {
		const char *query = R"(SELECT e_flags, e_uid FROM fannst.email_shortcuts 
		WHERE e_domain=? AND e_owners_uuid=? AND e_mailbox=?)";
		MailboxStatus res;
		cass_bool_t hasMorePages;
		int32_t largestUID = 0;

		res.s_Bucket = bucket;
		res.s_Domain = domain;
		res.s_UUID = uuid;

		// Gets the mailbox itself, to set the flags and message count
		//  because this is already stored inside of cassandra

		Mailbox mailbox = Mailbox::get(cassandra, bucket, domain, uuid, mailboxPath);
		res.s_Total = mailbox.e_MessageCount;
		res.s_Flags = mailbox.e_Flags;

		// Prepares the statement, we will not execute it yet because
		//  this will be done with pages, and not a single one

		CassStatement *statement = cass_statement_new(query, 3);
		DEFER(cass_statement_free(statement));
		cass_statement_bind_string(statement, 0, domain.c_str());
		cass_statement_bind_uuid(statement, 1, uuid);
		cass_statement_bind_string(statement, 2, mailboxPath.c_str());
		cass_statement_set_paging_size(statement, 50);

		do {
			// Executes the query and checks for initial errors, if there are some
			//  we throw an database exception

			CassFuture *future = cass_session_execute(cassandra->c_Session, statement);
			DEFER(cass_future_free(future));
			cass_future_wait(future);

			if (cass_future_error_code(future) != CASS_OK) {
				string error = "cass_session_execute() failed: ";
				error += CassandraConnection::getError(future);
				throw DatabaseException(error);
			}

			// Starts looping over the result, and building the final status
			//  instance, which will later be stored in redis

			const CassResult *result = cass_future_get_result(future);
			CassIterator *iterator = cass_iterator_from_result(result);
			DEFER(cass_iterator_free(iterator));

			while (cass_iterator_next(iterator)) {
				const CassRow *row = cass_iterator_get_row(iterator);
				int32_t flags, uid;

				cass_value_get_int32(cass_row_get_column_by_name(row, "e_flags"), &flags);
				cass_value_get_int32(cass_row_get_column_by_name(row, "e_uid"), &uid);

				if (!(BINARY_COMPARE(flags, _EMAIL_FLAG_SEEN))) {
					++res.s_Unseen;
				}

				if (uid > largestUID) largestUID = uid;
				++res.s_Total;
			}

			hasMorePages = cass_result_has_more_pages(result);
			if (hasMorePages) {
				cass_statement_set_paging_state(statement, result);
			}
		} while (hasMorePages);

		// Sets the next uuid as the largest one incremented
		//  after this we return the constructed status object

		res.s_NextUID = ++largestUID;
		return res;
	}

	int32_t MailboxStatus::addOneMessage(
		RedisConnection *redis,
		CassandraConnection *cassandra,
		const int64_t s_Bucket,
		const string &s_Domain,
		const CassUuid &uuid,
		const string &mailboxPath
	) {
		int32_t retUID;

		// Gets the old message status, and increments
		//  the next uuid, the total, the recent and unseen
		//  these are all effected when a new message is received 

		MailboxStatus old = MailboxStatus::get(redis, cassandra, s_Bucket, s_Domain, uuid, mailboxPath);
		retUID = old.s_NextUID;
		old.s_NextUID++;
		old.s_Total++;
		old.s_Recent++;
		old.s_Unseen++;

		// Updates the specified values, these will not be updated in cassandra, but only inside
		//  redis, since cassandra is not meant for so many updates

		char prefix[512], command[1024];
		getPrefix(s_Bucket, s_Domain.c_str(), uuid, mailboxPath.c_str(), prefix);
		sprintf(
			command, "%s %s v2 %d v3 %d v5 %d v6 %d",
			"HMSET", prefix,
			old.s_Unseen, old.s_Total, old.s_NextUID, old.s_Recent
		);

		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command
		));
		DEFER(freeReplyObject(reply));

		if (reply->type == REDIS_REPLY_ERROR) {
			string error = "redisCommand() failed: ";
			error += string(reply->str, reply->len);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}

		return retUID;
	}
}