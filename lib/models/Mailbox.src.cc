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
	Mailbox::Mailbox(void) noexcept {}

	Mailbox::Mailbox(
		const int64_t e_Bucket, const string &e_Domain,
		const CassUuid &e_UUID, const string &e_MailboxPath,
		const bool e_MailboxStand, const int32_t e_MessageCount,
		const int32_t e_Flags, const bool e_Subscribed
	) noexcept:
		e_Bucket(e_Bucket), e_Domain(e_Domain), e_UUID(e_UUID),
		e_MailboxPath(e_MailboxPath), e_MailboxStand(e_MailboxStand),
		e_MessageCount(e_MessageCount), e_Flags(e_Flags), e_Subscribed(e_Subscribed)
	{}

	void Mailbox::save(CassandraConnection *cassandra) {
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

		// Creates the statement, binds the values and executes it, if something goes
		//  wrong we throw an database exception

		statement = cass_statement_new(query, 8);
		DEFER(cass_statement_free(statement));
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
		DEFER(cass_future_free(future));
		cass_future_wait(future);

		if (cass_future_error_code(future) != CASS_OK) {
			string error = "cass_session_execute() failed: ";
			error += CassandraConnection::getError(future);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}
	}

	vector<Mailbox> Mailbox::gatherAll(
		CassandraConnection *cassandra, const int64_t bucket,
		const string &domain, const CassUuid &uuid,
		const bool subscribedOnly
	) {
		const char *query = nullptr;
		if (subscribedOnly) {
			query = R"(SELECT e_mailbox_path, e_mailbox_stand, e_message_count,
				e_flags, e_subscribed FROM fannst.mailboxes WHERE e_bucket=? AND e_domain=? AND e_uuid=? AND e_subscribed=? ALLOW FILTERING)";
		} else {
			query = R"(SELECT e_mailbox_path, e_mailbox_stand, e_message_count,
				e_flags, e_subscribed FROM fannst.mailboxes WHERE e_bucket=? AND e_domain=? AND e_uuid=?)";
		}

		CassStatement *statement = nullptr;
		CassFuture *future = nullptr;
		vector<Mailbox> ret;

		// =================================
		// Executes the query
		// =================================

		statement = cass_statement_new(query, (subscribedOnly ? 4 : 3));
		DEFER(cass_statement_free(statement));
		cass_statement_bind_int64(statement, 0, bucket);
		cass_statement_bind_string(statement, 1, domain.c_str());
		cass_statement_bind_uuid(statement, 2, uuid);

		if (subscribedOnly) {
			cass_statement_bind_bool(statement, 3, cass_true);
		}

		future = cass_session_execute(cassandra->c_Session, statement);
		DEFER(cass_future_free(future));
		cass_future_wait(future);

		if (cass_future_error_code(future) != CASS_OK) {
			string error = "cass_session_execute() failed: ";
			error += CassandraConnection::getError(future);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}

		// =================================
		// Gets the result
		// =================================

		const CassResult *result = cass_future_get_result(future);
		CassIterator *iterator = cass_iterator_from_result(result);
		DEFER_M({
			cass_result_free(result);
			cass_iterator_free(iterator);
		});

		while (cass_iterator_next(iterator)) {
			const CassRow *row = cass_iterator_get_row(iterator);
			const char *mailboxPath = nullptr;
			size_t mailboxPathLen;
			Mailbox mailbox;
			cass_bool_t subscribed, mailboxStand;

			cass_value_get_string(cass_row_get_column_by_name(row, "e_mailbox_path"), &mailboxPath, &mailboxPathLen);
			cass_value_get_bool(cass_row_get_column_by_name(row, "e_mailbox_stand"), &mailboxStand);
			cass_value_get_bool(cass_row_get_column_by_name(row, "e_subscribed"), &subscribed);
			cass_value_get_int32(cass_row_get_column_by_name(row, "e_flags"), &mailbox.e_Flags);
			cass_value_get_int32(cass_row_get_column_by_name(row, "e_message_count"), &mailbox.e_MessageCount);

			mailbox.e_Subscribed = (subscribed == cass_true ? true : false);
			mailbox.e_MailboxStand = (mailboxStand == cass_true ? true : false);
			mailbox.e_MailboxPath.append(mailboxPath, mailboxPathLen);

			ret.push_back(mailbox);
		}

		return ret;
	}

	Mailbox Mailbox::get(
		CassandraConnection *cassandra, const int64_t bucket,
		const string &domain, const CassUuid &uuid,
		const string &mailboxPath
	) {
		const char *query = R"(SELECT e_mailbox_stand, e_subscribed, e_flags, e_message_count 
		FROM fannst.mailboxes
		WHERE e_bucket=? AND e_domain=? AND e_uuid=? AND e_mailbox_path=?)";
		Mailbox res;
		CassFuture *future = nullptr;
		CassStatement *statement = nullptr;

		// ===================================
		// Prepares and executes
		// ===================================

		statement = cass_statement_new(query, 4);
		DEFER(cass_statement_free(statement));
		cass_statement_bind_int64(statement, 0, bucket);
		cass_statement_bind_string(statement, 1, domain.c_str());
		cass_statement_bind_uuid(statement, 2, uuid);
		cass_statement_bind_string(statement, 3, mailboxPath.c_str());

		future = cass_session_execute(cassandra->c_Session, statement);
		DEFER(cass_future_free(future));
		cass_future_wait(future);

		if (cass_future_error_code(future) != CASS_OK) {
			string error = "cass_session_execute() failed: ";
			error += CassandraConnection::getError(future);
			throw DatabaseException(EXCEPT_DEBUG(error));
		}

		// ===================================
		// Gets the stuff
		// ===================================

		const CassResult *result = cass_future_get_result(future);
		DEFER(cass_result_free(result));
		const CassRow *row = cass_result_first_row(result);

		if (!row) {
			throw EmptyQuery(EXCEPT_DEBUG("Could not find Mailbox"));
		}

		res.e_Domain = domain;
		res.e_Bucket = bucket;
		res.e_UUID = uuid;
		res.e_MailboxPath = mailboxPath;

		cass_bool_t subscribed, mailboxStand;
		cass_value_get_int32(cass_row_get_column_by_name(row, "e_flags"), &res.e_Flags);
		cass_value_get_int32(cass_row_get_column_by_name(row, "e_message_count"), &res.e_MessageCount);
		cass_value_get_bool(cass_row_get_column_by_name(row, "e_subscribed"), &subscribed);
		cass_value_get_bool(cass_row_get_column_by_name(row, "e_mailbox_stand"), &mailboxStand);

		res.e_Subscribed = (subscribed == cass_true ? true : false);
		res.e_MailboxStand = (mailboxStand == cass_true ? true : false);

		return res;
	}
}