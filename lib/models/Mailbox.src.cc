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

#include "Mailbox.src.h"

namespace FSMTP::Models
{
	/**
	 * Default empty constructor for MailBox
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	Mailbox::Mailbox(void) noexcept
	{}

	/**
	 * The constructor which litterally makes one
	 *
	 * @Param {const int32_t} e_Bucket
	 * @Param {const std::string &} e_Domain
	 * @Param {const CassUuid &} e_UUID
	 * @Param {const std::String &} e_MailboxPath;
	 * @Param {const bool} e_MailboxStand
	 * @Param {const in64_t} e_MessageCount
	 * @Param {const int32_t} e_Flags
	 * @Param {const bool} e_Subscribed
	 * @Return {void}
	 */
	Mailbox::Mailbox(
		const int64_t e_Bucket,
		const std::string &e_Domain,
		const CassUuid &e_UUID,
		const std::string &e_MailboxPath,
		const bool e_MailboxStand,
		const int32_t e_MessageCount,
		const int32_t e_Flags,
		const bool e_Subscribed
	) noexcept:
		e_Bucket(e_Bucket), e_Domain(e_Domain), e_UUID(e_UUID),
		e_MailboxPath(e_MailboxPath), e_MailboxStand(e_MailboxStand),
		e_MessageCount(e_MessageCount), e_Flags(e_Flags), e_Subscribed(e_Subscribed)
	{}

	/**
	 * Saves an mailbox to the database
	 *
	 * @Param {CassandraConnection *} cassandra
	 * @Return {void}
	 */
	void Mailbox::save(CassandraConnection *cassandra)
	{
		const char *query = R"(INSERT INTO fannst.mailboxes (
			e_bucket, e_domain, e_uuid,
			e_mailbox_path, e_mailbox_stand, e_message_count,
			e_flags, e_subscribed
		) VALUES (
			?, ?, ?,
			?, ?, ?,
			?, ?
		))";
		CassStatement *statement = nullptr;
		CassFuture *future = nullptr;
		CassError rc;

		statement = cass_statement_new(query, 8);
		cass_statement_bind_int64(statement, 0, this->e_Bucket);
		cass_statement_bind_string(statement, 1, this->e_Domain.c_str());
		cass_statement_bind_uuid(statement, 2, this->e_UUID);
		cass_statement_bind_string(statement, 3, this->e_MailboxPath.c_str());
		cass_statement_bind_bool(
			statement, 4, 
			(this->e_MailboxStand ? cass_true : cass_false)
		);
		cass_statement_bind_int32(statement, 5, this->e_MessageCount);
		cass_statement_bind_int32(statement, 6, this->e_Flags);
		cass_statement_bind_bool(
			statement, 7, 
			(this->e_Subscribed ? cass_true : cass_false)
		);

		future = cass_session_execute(cassandra->c_Session, statement);
		cass_future_wait(future);

		rc = cass_future_error_code(future);
		if (rc != CASS_OK)
		{
			std::string error = "cass_session_execute() failed: ";
			error += CassandraConnection::getError(future);
			cass_future_free(future);
			cass_statement_free(statement);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}

		cass_future_free(future);
		cass_statement_free(statement);
	}

	/**
	 * Gathers all mailboxes from an user
	 *
	 * @Param {CassandraConnection *} cassandra
	 * @Param {const int64_t} bucket
	 * @Param {const std::string &} domain
	 * @Param {CassUuid &} uuid
	 * @Param {const bool} subscribedOnly
	 * @Return {std::vector<Mailbox>}
	 */
	std::vector<Mailbox> Mailbox::gatherAll(
		CassandraConnection *cassandra,
		const int64_t bucket,
		const std::string &domain,
		const CassUuid &uuid,
		const bool subscribedOnly
	)
	{
		const char *query = nullptr;
		if (subscribedOnly)
		{
			query = R"(SELECT e_mailbox_path, e_mailbox_stand, e_message_count,
				e_flags, e_subscribed FROM fannst.mailboxes WHERE e_bucket=? AND e_domain=? AND e_uuid=? AND e_subscribed=? ALLOW FILTERING)";
		} else
		{
			query = R"(SELECT e_mailbox_path, e_mailbox_stand, e_message_count,
				e_flags, e_subscribed FROM fannst.mailboxes WHERE e_bucket=? AND e_domain=? AND e_uuid=?)";
		}

		CassStatement *statement = nullptr;
		CassFuture *future = nullptr;
		std::vector<Mailbox> ret;
		CassError rc;

		// =================================
		// Executes the query
		//
		// Creates the statement and exe
		// - cutes it
		// =================================

		statement = cass_statement_new(query, (subscribedOnly ? 4 : 3));
		cass_statement_bind_int64(statement, 0, bucket);
		cass_statement_bind_string(statement, 1, domain.c_str());
		cass_statement_bind_uuid(statement, 2, uuid);
		if (subscribedOnly) cass_statement_bind_bool(statement, 3, cass_true);

		future = cass_session_execute(cassandra->c_Session, statement);
		cass_future_wait(future);

		rc = cass_future_error_code(future);
		if (rc != CASS_OK)
		{
			std::string error = "cass_session_execute() failed: ";
			error += CassandraConnection::getError(future);
			cass_future_free(future);
			cass_statement_free(statement);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}

		// =================================
		// Gets the result
		//
		// Processes the values
		// =================================

		const CassResult *result = cass_future_get_result(future);
		CassIterator *iterator = cass_iterator_from_result(result);

		while (cass_iterator_next(iterator))
		{
			const CassRow *row = cass_iterator_get_row(iterator);
			const char *mailboxPath = nullptr;
			std::size_t mailboxPathLen;
			Mailbox mailbox;

			cass_value_get_string(
				cass_row_get_column_by_name(row, "e_mailbox_path"),
				&mailboxPath, &mailboxPathLen
			);
			cass_value_get_bool(
				cass_row_get_column_by_name(row, "e_mailbox_stand"),
				reinterpret_cast<cass_bool_t *>(&mailbox.e_MailboxStand)
			);
			cass_value_get_bool(
				cass_row_get_column_by_name(row, "e_subscribed"),
				reinterpret_cast<cass_bool_t *>(&mailbox.e_Subscribed)
			);
			cass_value_get_int32(
				cass_row_get_column_by_name(row, "e_flags"),
				&mailbox.e_Flags
			);
			cass_value_get_int32(
				cass_row_get_column_by_name(row, "e_message_count"),
				&mailbox.e_MessageCount
			);

			mailbox.e_MailboxPath.append(mailboxPath, mailboxPathLen);

			ret.push_back(mailbox);
		}

		cass_result_free(result);
		cass_iterator_free(iterator);
		cass_statement_free(statement);
		cass_future_free(future);
		return ret;
	}

	/**
	 * Gets an mailbox
	 *
	 * @Param {CassandraConnection *} cassandra
	 * @Param {const std::string &} domain
	 * @Param {const CassUuid &} uuid
	 * @Param {const std::string &} mailbox
	 * @Return {MailboxStatus}
	 */
	Mailbox Mailbox::get(
		CassandraConnection *cassandra,
		const int64_t bucket,
		const std::string &domain,
		const CassUuid &uuid,
		const std::string &mailboxPath
	)
	{
		Mailbox res;
		const char *query = R"(SELECT e_mailbox_stand, e_subscribed, e_flags, e_message_count 
		FROM fannst.mailboxes
		WHERE e_bucket=? AND e_domain=? AND e_uuid=? AND e_mailbox_path=?)";
		CassFuture *future = nullptr;
		CassStatement *statement = nullptr;
		CassError rc;

		// ===================================
		// Prepares and executes
		// 
		// Prepares the statement, binds the 
		// - values and executes
		// ===================================

		statement = cass_statement_new(query, 4);
		cass_statement_bind_int64(statement, 0, bucket);
		cass_statement_bind_string(statement, 1, domain.c_str());
		cass_statement_bind_uuid(statement, 2, uuid);
		cass_statement_bind_string(statement, 3, mailboxPath.c_str());

		future = cass_session_execute(cassandra->c_Session, statement);
		cass_future_wait(future);

		rc = cass_future_error_code(future);
		if (rc != CASS_OK)
		{
			std::string error = "cass_session_execute() failed: ";
			error += CassandraConnection::getError(future);
			cass_future_free(future);
			cass_statement_free(statement);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}

		// ===================================
		// Gets the stuff
		//
		// Gets the values from the database
		// ===================================

		const CassResult *result = cass_future_get_result(future);
		const CassRow *row = cass_result_first_row(result);

		if (!row)
		{
			cass_future_free(future);
			cass_result_free(result);
			cass_statement_free(statement);
			throw EmptyQuery(EXCEPT_DEBUG("Could not find Mailbox"));
		}

		res.e_Domain = domain;
		res.e_Bucket = bucket;
		res.e_UUID = uuid;
		res.e_MailboxPath = mailboxPath;

		cass_value_get_int32(
			cass_row_get_column_by_name(row, "e_flags"), 
			&res.e_Flags
		);
		cass_value_get_int32(
			cass_row_get_column_by_name(row, "e_message_count"), 
			&res.e_MessageCount
		);
		cass_value_get_bool(
			cass_row_get_column_by_name(row, "e_subscribed"),
			reinterpret_cast<cass_bool_t *>(&res.e_Subscribed)
		);
		cass_value_get_bool(
			cass_row_get_column_by_name(row, "e_mailbox_stand"),
			reinterpret_cast<cass_bool_t *>(&res.e_MailboxStand)
		);

		cass_future_free(future);
		cass_result_free(result);
		cass_statement_free(statement);
		return res;
	}
}