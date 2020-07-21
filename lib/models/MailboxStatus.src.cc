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
		res.s_UUID = cass_uuid_timestamp(uuid);
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
					if (res.s_Recent == 0) res.s_Recent = uid;
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
}