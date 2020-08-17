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

#include "EmailShortcut.src.h"

namespace FSMTP::Models
{
  EmailShortcut::EmailShortcut(void):
    e_Flags(0)
  {}

  void EmailShortcut::save(CassandraConnection *cassandra) {
    const char *query = R"(INSERT INTO fannst.email_shortcuts (
      e_domain, e_subject, e_preview,
      e_owners_uuid, e_email_uuid, e_bucket,
      e_mailbox, e_size_octets, e_uid,
      e_flags, e_from
    ) VALUES (
      ?, ?, ?,
      ?, ?, ?,
      ?, ?, ?,
      ?, ?
    ))";
    CassStatement *statement = nullptr;
    CassFuture *future = nullptr;

    // Creates the statement and binds the instance variables to the query
    //  after which we insert the data into cassandra

    statement = cass_statement_new(query, 11);
    DEFER(cass_statement_free(statement));
    cass_statement_bind_string(statement, 0, this->e_Domain.c_str());
    cass_statement_bind_string(statement, 1, this->e_Subject.c_str());
    cass_statement_bind_string(statement, 2, this->e_Preview.c_str());
    cass_statement_bind_uuid(statement, 3, this->e_OwnersUUID);
    cass_statement_bind_uuid(statement, 4, this->e_EmailUUID);
    cass_statement_bind_int64(statement, 5, this->e_Bucket);
    cass_statement_bind_string(statement, 6, this->e_Mailbox.c_str());
    cass_statement_bind_int64(statement, 7, this->e_SizeOctets);
    cass_statement_bind_int32(statement, 8, this->e_UID);
    cass_statement_bind_int32(statement, 9, this->e_Flags);
    cass_statement_bind_string(statement, 10, this->e_From.c_str());

    future = cass_session_execute(cassandra->c_Session, statement);
    DEFER(cass_future_free(future));
    cass_future_wait(future);

    if (cass_future_error_code(future) != CASS_OK) {
      string error = "cass_session_execute() failed: ";
      error += CassandraConnection::getError(future);
      throw DatabaseException(EXCEPT_DEBUG(error));
    }
  }

  vector<EmailShortcut> EmailShortcut::gatherAll(
    CassandraConnection *cassandra, const int32_t skip,
    int32_t limit, const string &domain,
    const string &mailbox, const CassUuid &uuid
  ) {
    vector<EmailShortcut> ret = {};

    const char *query = "SELECT * FROM fannst.email_shortcuts WHERE e_domain=? AND e_mailbox=? AND e_owners_uuid=? LIMIT ?";
    CassStatement *statement = nullptr;
    CassFuture *future = nullptr;
    CassError rc;

    // Limit of 80 emails a time
    if (limit > 80) limit = 80;

    // =======================================
    // Performs the query
    // =======================================

    statement = cass_statement_new(query, 4);
    DEFER(cass_statement_free(statement));
    cass_statement_bind_string(statement, 0, domain.c_str());
    cass_statement_bind_string(statement, 1, mailbox.c_str());
    cass_statement_bind_uuid(statement, 2, uuid);
    cass_statement_bind_int32(statement, 3, limit);

    future = cass_session_execute(cassandra->c_Session, statement);
    DEFER(cass_future_free(future));
    cass_future_wait(future);

    if (cass_future_error_code(future) != CASS_OK) {
      string error = "cass_session_execute() failed: ";
      error += CassandraConnection::getError(future);
      throw DatabaseException(EXCEPT_DEBUG(error));
    }

    // =======================================
    // Handles the data
    // =======================================

    const CassResult *result = cass_future_get_result(future);
    CassIterator *resultIterator = cass_iterator_from_result(result);
    DEFER_M({
      cass_result_free(result);
      cass_iterator_free(resultIterator);
    });

    while(cass_iterator_next(resultIterator)) {
      const CassRow *row = cass_iterator_get_row(resultIterator);
      EmailShortcut shortcut;
      const char *domain, *subject, *preview, *mailbox, *from;
      domain = subject = preview = mailbox = from = nullptr;
      size_t domainLen, subjectLen, previewLen, mailboxLen, fromLen;

      // Gets the values from the result, and puts them into the variables
      
      cass_value_get_string(cass_row_get_column_by_name(row, "e_domain"), &domain, &domainLen);
      cass_value_get_string(cass_row_get_column_by_name(row, "e_subject"), &subject, &subjectLen);
      cass_value_get_string(cass_row_get_column_by_name(row, "e_preview"), &preview, &previewLen);
      cass_value_get_uuid(cass_row_get_column_by_name(row, "e_owners_uuid"), &shortcut.e_OwnersUUID);
      cass_value_get_uuid(cass_row_get_column_by_name(row, "e_email_uuid"), &shortcut.e_EmailUUID);
      cass_value_get_int64(cass_row_get_column_by_name(row, "e_owners_uuid"), &shortcut.e_Bucket);
      cass_value_get_string(cass_row_get_column_by_name(row, "e_mailbox"), &mailbox, &mailboxLen);
      cass_value_get_int64(cass_row_get_column_by_name(row, "e_size_octets"), &shortcut.e_SizeOctets);
      cass_value_get_int32(cass_row_get_column_by_name(row, "e_uid"), &shortcut.e_UID);
      cass_value_get_int32(cass_row_get_column_by_name(row, "e_Flags"), &shortcut.e_Flags);
      cass_value_get_string(cass_row_get_column_by_name(row, "e_from"), &from, &fromLen);

      // Turns the buffers into real strings, and starts appending
      //  them to the current shortcut, this is done with the length
      //  returned by the get string of cassandra. After this we push
      //  the result in to the vector

      shortcut.e_Domain.append(domain, domainLen);
      shortcut.e_Preview.append(preview, previewLen);
      shortcut.e_Subject.append(subject, subjectLen);
      shortcut.e_Mailbox.append(mailbox, mailboxLen);
      shortcut.e_From.append(from, fromLen);

      ret.push_back(shortcut);
    }

    return ret;
  }

  pair<int64_t, size_t> EmailShortcut::getStat(
    CassandraConnection *cassandra, const int32_t skip,
    int32_t limit, const string &domain,
    const string &mailbox, const CassUuid &uuid
  ) {
    size_t total = 0;
    int64_t octets = 0;

    const char *query = R"(SELECT e_size_octets 
    FROM fannst.email_shortcuts 
    WHERE e_domain=? AND e_mailbox=? AND e_owners_uuid=? 
    LIMIT ?)";
    CassStatement *statement = nullptr;
    cass_bool_t hasMorePages = cass_false;

    // Limit of 500 emails a time
    if (limit > 500) limit = 500;

    // =======================================
    // Prepares the statement
    // =======================================

    statement = cass_statement_new(query, 4);
    DEFER(cass_statement_free(statement));
    cass_statement_bind_string(statement, 0, domain.c_str());
    cass_statement_bind_string(statement, 1, mailbox.c_str());
    cass_statement_bind_uuid(statement, 2, uuid);
    cass_statement_bind_int32(statement, 3, limit);
    cass_statement_set_paging_size(statement, 20);

    // =======================================
    // Counts the data
    // =======================================

    do {
      CassFuture *future = nullptr;

      // Executes the query, and checks for errors
      future = cass_session_execute(cassandra->c_Session, statement);
      DEFER(cass_future_free(future));
      cass_future_wait(future);

      if (cass_future_error_code(future) != CASS_OK) {
        string message = "cass_session_execute() failed: ";
        message += CassandraConnection::getError(future);

        throw DatabaseException(EXCEPT_DEBUG(message));
      }

      const CassResult *result = cass_future_get_result(future);
      CassIterator *iterator = cass_iterator_from_result(result);
      DEFER_M({
        cass_result_free(result);
        cass_iterator_free(iterator);
      });

      while (cass_iterator_next(iterator)) {
        const CassRow *row = cass_iterator_get_row(iterator);

        // Gets the size of the raw message
        int64_t totalOctets;
        cass_value_get_int64(cass_row_get_column_by_name(
          row, "e_size_octets"),
          &totalOctets
        );

        total++;
        octets += totalOctets;
      }

      hasMorePages = cass_result_has_more_pages(result);
      if (hasMorePages) {
        cass_statement_set_paging_state(statement, result);
      }
    } while (hasMorePages);

    return make_pair(octets, total);
  }

  vector<tuple<CassUuid, int64_t, int64_t>> EmailShortcut::gatherAllReferencesWithSize(
    CassandraConnection *cassandra, const int32_t skip,
    int32_t limit, const string &domain,
    const string &mailbox, const CassUuid &uuid,
    const bool deleted
  ) {
    vector<tuple<CassUuid, int64_t, int64_t>>  ret = {};

    const char *query = R"(SELECT e_size_octets, e_email_uuid, e_bucket, e_flags 
    FROM fannst.email_shortcuts 
    WHERE e_domain=? AND e_mailbox=? AND e_owners_uuid=?)";
    CassStatement *statement = nullptr;
    cass_bool_t hasMorePages = cass_false;

    // Limit of 500 emails a time
    if (limit < 1) throw runtime_error("Limit must be positive");
    if (limit > 500) limit = 500;

    // =======================================
    // Prepares the statement
    // =======================================

    statement = cass_statement_new(query, 3);
    DEFER(cass_statement_free(statement));
    cass_statement_bind_string(statement, 0, domain.c_str());
    cass_statement_bind_string(statement, 1, mailbox.c_str());
    cass_statement_bind_uuid(statement, 2, uuid);
    cass_statement_set_paging_size(statement, 20);

    // =======================================
    // Counts the data
    // =======================================

    do {
      CassFuture *future = nullptr;

      future = cass_session_execute(cassandra->c_Session, statement);
      DEFER(cass_future_free(future));
      cass_future_wait(future);

      if (cass_future_error_code(future) != CASS_OK) {
        string message = "cass_session_execute() failed: ";
        message += CassandraConnection::getError(future);

        cass_future_free(future);
        cass_statement_free(statement);
        throw DatabaseException(EXCEPT_DEBUG(message));
      }

      // Starts iterating over the results, since an user can specify deleted as true
      //  or false, we will compare the flags to check if we want to process the current
      //  entry or just skip, next to that before continueing we also check for
      //  the limit, if the current count is bigger then the limit, just ignore

      const CassResult *result = cass_future_get_result(future);
      CassIterator *iterator = cass_iterator_from_result(result);
      DEFER_M({
        cass_result_free(result);
        cass_iterator_free(iterator);
      });

      while (cass_iterator_next(iterator)) {
        const CassRow *row = cass_iterator_get_row(iterator);
        int64_t octets, bucket;
        int32_t flags;
        CassUuid uuid;

        // Compares the flags to check if the message is marked as deleted
        //  this will then be checked with the deleted boolean specified in
        //  the arguments of this method

        cass_value_get_int32(cass_row_get_column_by_name(row, "e_flags"), &flags);
        if ((flags & _EMAIL_FLAG_DELETED) == _EMAIL_FLAG_DELETED) {
          if (!deleted) continue;
        } else if (deleted) {
          continue;
        }

        // Gets the other values such as the size in octets, the bucket
        //  email uuid. After this we make an puple and push it to the result

        cass_value_get_int64(cass_row_get_column_by_name(row, "e_size_octets"), &octets);
        cass_value_get_int64(cass_row_get_column_by_name(row, "e_bucket"), &bucket);
        cass_value_get_uuid(cass_row_get_column_by_name(row, "e_email_uuid"), &uuid);

        ret.push_back(tuple<CassUuid, int64_t, int64_t>(uuid, octets, bucket));
      }

      hasMorePages = cass_result_has_more_pages(result);
      if (hasMorePages) {
        cass_statement_set_paging_state(statement, result);
      }
    } while (hasMorePages);

    reverse(ret.begin(), ret.end());
    return ret;
  }

  void EmailShortcut::deleteOne(
    CassandraConnection *cassandra, const string &domain,
    const CassUuid &ownersUuid, const CassUuid &emailUuid,
    const string &mailbox
  ) {
    const char *query = R"(DELETE FROM fannst.email_shortcuts 
    WHERE e_domain=? AND e_type=? AND e_owners_uuid=? AND e_email_uuid=? AND e_mailbox=?)";
    CassStatement *statement = nullptr;
    CassFuture *future = nullptr;

    statement = cass_statement_new(query, 5);
    DEFER(cass_statement_free(statement));
    cass_statement_bind_string(statement, 0, domain.c_str());
    cass_statement_bind_int32(statement, 1, EmailType::ET_INCOMMING);
    cass_statement_bind_uuid(statement, 2, ownersUuid);
    cass_statement_bind_uuid(statement, 3, emailUuid);
    cass_statement_bind_string(statement, 4, mailbox.c_str());

    future = cass_session_execute(cassandra->c_Session, statement);
    DEFER(cass_future_free(future));
    cass_future_wait(future);

    if (cass_future_error_code(future) != CASS_OK) {
      string error = "Could not delete email shortcut: ";
      error += CassandraConnection::getError(future);
      throw DatabaseException(error);
    }
  }
}
