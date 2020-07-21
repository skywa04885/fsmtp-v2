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
  /**
   * Default empty constructor for the EmailShortcut class
   *
   * @Param {void}
   * @Return {void}
   */
  EmailShortcut::EmailShortcut(void)
  {}

  /**
   * Stores an email shortcut in the cassandra database
   *
   * @Param {CassandraConnection *} cassandra
   * @Return {void}
   */
  void EmailShortcut::save(CassandraConnection *cassandra)
  {
    const char *query = R"(INSERT INTO fannst.email_shortcuts (
      e_domain, e_subject, e_preview,
      e_owners_uuid, e_email_uuid, e_bucket,
      e_mailbox, e_size_octets, e_uid,
      e_flags
    ) VALUES (
      ?, ?, ?,
      ?, ?, ?,
      ?, ?, ?,
      ?
    ))";
    CassStatement *statement = nullptr;
    CassFuture *future = nullptr;
    CassError rc;

    // Creates the statement and query, then binds
    // - the values
    std::cout << this->e_Domain.c_str() << std::endl;
    statement = cass_statement_new(query, 10);
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

    // Executes the query and checks for errors
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

    // Frees the memory and returns
    cass_statement_free(statement);
    cass_future_free(future);
  }

  /**
   * Gathers all messages from an specific user
   *
   * @Param {CassandraConnection *} cassandra
   * @Param {const int32_t} skip
   * @Param {int32_t} limit
   * @Param {const std::string &} domain
   * @Param {const std::string &mailbox}
   * @Param {const CassUuid &} uuid
   * @Return {std::vector<EmailShortcut>}
   */
  std::vector<EmailShortcut> EmailShortcut::gatherAll(
    CassandraConnection *cassandra,
    const int32_t skip,
    int32_t limit,
    const std::string &domain,
    const std::string &mailbox,
    const CassUuid &uuid
  )
  {
    std::vector<EmailShortcut> ret = {};

    const char *query = "SELECT * FROM fannst.email_shortcuts WHERE e_domain=? AND e_mailbox=? AND e_owners_uuid=? LIMIT ?";
    CassStatement *statement = nullptr;
    CassFuture *future = nullptr;
    CassError rc;

    // Limit of 80 emails a time
    if (limit > 80) limit = 80;

    // =======================================
    // Performs the query
    //
    // Prepares and executes the query
    // =======================================

    // Prepares the statement and puts the values into it
    statement = cass_statement_new(query, 4);
    cass_statement_bind_string(statement, 0, domain.c_str());
    cass_statement_bind_string(statement, 1, mailbox.c_str());
    cass_statement_bind_uuid(statement, 2, uuid);
    cass_statement_bind_int32(statement, 3, limit);

    // Executes the query and checks for errors
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

    // =======================================
    // Handles the data
    //
    // Handles the data from the database
    // - and puts it into the result vector
    // =======================================

    // Gets the result, and starts looping
    const CassResult *result = cass_future_get_result(future);
    CassIterator *resultIterator = cass_iterator_from_result(result);

    while(cass_iterator_next(resultIterator))
    {
      // Prepares memory for the new element
      const CassRow *row = cass_iterator_get_row(resultIterator);
      const char *domain = nullptr;
      std::size_t domainLen;
      const char *subject = nullptr;
      std::size_t subjectLen;
      const char *preview = nullptr;
      std::size_t previewLen;
      const char *mailbox = nullptr;
      std::size_t mailboxLen;
      EmailShortcut shortcut;

      // Gets the values
      cass_value_get_string(
        cass_row_get_column_by_name(row, "e_domain"),
        &domain,
        &domainLen
      );
      cass_value_get_string(
        cass_row_get_column_by_name(row, "e_subject"),
        &subject,
        &subjectLen
      );
      cass_value_get_string(
        cass_row_get_column_by_name(row, "e_preview"),
        &preview,
        &previewLen
      );
      cass_value_get_uuid(
        cass_row_get_column_by_name(row, "e_owners_uuid"),
        &shortcut.e_OwnersUUID
      );
      cass_value_get_uuid(
        cass_row_get_column_by_name(row, "e_email_uuid"),
        &shortcut.e_EmailUUID
      );
      cass_value_get_int64(
        cass_row_get_column_by_name(row, "e_owners_uuid"),
        &shortcut.e_Bucket
      );
      cass_value_get_string(
        cass_row_get_column_by_name(row, "e_mailbox"),
        &mailbox, &mailboxLen
      );
      cass_value_get_int64(
        cass_row_get_column_by_name(row, "e_size_octets"),
        &shortcut.e_SizeOctets
      );
      cass_value_get_int32(
        cass_row_get_column_by_name(row, "e_uid"),
        &shortcut.e_UID
      );
      cass_value_get_int32(
        cass_row_get_column_by_name(row, "e_Flags"),
        &shortcut.e_Flags
      );

      // Stores the strings
      shortcut.e_Domain.append(domain, domainLen);
      shortcut.e_Preview.append(preview, previewLen);
      shortcut.e_Subject.append(subject, subjectLen);
      shortcut.e_Mailbox.append(mailbox, mailboxLen);

      // Free's the memory and pushes the result
      ret.push_back(shortcut);
    }

    // Frees the memory
    cass_result_free(result);
    cass_iterator_free(resultIterator);
    cass_future_free(future);

    // Returns the vector
    return ret;
  }

  std::pair<int64_t, std::size_t> EmailShortcut::getStat(
    CassandraConnection *cassandra,
    const int32_t skip,
    int32_t limit,
    const std::string &domain,
    const std::string &mailbox,
    const CassUuid &uuid
  )
  {
    std::size_t total = 0;
    int64_t octets = 0;

    const char *query = "SELECT e_size_octets FROM fannst.email_shortcuts WHERE e_domain=? AND e_mailbox=? AND e_owners_uuid=? LIMIT ?";
    CassStatement *statement = nullptr;
    cass_bool_t hasMorePages = cass_false;
    CassError rc;

    // Limit of 80 emails a time
    if (limit > 500) limit = 500;

    // =======================================
    // Prepares the statement
    //
    // Prepares the statement and binds the
    // - values
    // =======================================

    // Prepares the statement and binds the values
    statement = cass_statement_new(query, 4);
    cass_statement_bind_string(statement, 0, domain.c_str());
    cass_statement_bind_string(statement, 1, mailbox.c_str());
    cass_statement_bind_uuid(statement, 2, uuid);
    cass_statement_bind_int32(statement, 3, limit);
    cass_statement_set_paging_size(statement, 20);

    // =======================================
    // Counts the data
    //
    // Counts the total octets and count
    // =======================================

    do
    {
      CassError rc;
      CassFuture *future = nullptr;

      // Executes the query, and checks for errors
      future = cass_session_execute(cassandra->c_Session, statement);
      cass_future_wait(future);

      rc = cass_future_error_code(future);
      if (rc != CASS_OK)
      {
        std::string message = "cass_session_execute() failed: ";
        message += CassandraConnection::getError(future);

        cass_future_free(future);
        cass_statement_free(statement);
        throw DatabaseException(EXCEPT_DEBUG(message));
      }

      // Creates the iterator and starts looping
      // - over the received data
      const CassResult *result = cass_future_get_result(future);
      CassIterator *iterator = cass_iterator_from_result(result);

      while (cass_iterator_next(iterator))
      {
        const CassRow *row = cass_iterator_get_row(iterator);

        // Gets the size of the raw message
        int64_t totalOctets;
        cass_value_get_int64(cass_row_get_column_by_name(
          row, "e_size_octets"),
          &totalOctets
        );

        // Increments the counters
        total++;
        octets += totalOctets;
      }

      // Performs the paging stuff
      hasMorePages = cass_result_has_more_pages(result);
      if (hasMorePages)
        cass_statement_set_paging_state(statement, result);

      // Frees the memory
      cass_result_free(result);
      cass_iterator_free(iterator);
      cass_future_free(future);
    } while (hasMorePages);

    // Frees the memory and returns
    cass_statement_free(statement);
    return std::make_pair(octets, total);
  }


  std::vector<std::tuple<CassUuid, int64_t, int64_t>> EmailShortcut::gatherAllReferencesWithSize(
    CassandraConnection *cassandra,
    const int32_t skip,
    int32_t limit,
    const std::string &domain,
    const std::string &mailbox,
    const CassUuid &uuid
  )
  {
    std::vector<std::tuple<CassUuid, int64_t, int64_t>>  ret = {};

    const char *query = "SELECT e_size_octets, e_email_uuid, e_bucket FROM fannst.email_shortcuts WHERE e_domain=? AND e_mailbox=? AND e_owners_uuid=? LIMIT ?";
    CassStatement *statement = nullptr;
    cass_bool_t hasMorePages = cass_false;
    CassError rc;

    // Limit of 80 emails a time
    if (limit < 1) throw std::runtime_error("Limit must be positive");
    if (limit > 500) limit = 500;

    // =======================================
    // Prepares the statement
    //
    // Prepares the statement and binds the
    // - values
    // =======================================

    // Prepares the statement and binds the values
    statement = cass_statement_new(query, 4);
    cass_statement_bind_string(statement, 0, domain.c_str());
    cass_statement_bind_string(statement, 1, mailbox.c_str());
    cass_statement_bind_uuid(statement, 2, uuid);
    cass_statement_bind_int32(statement, 3, limit);
    cass_statement_set_paging_size(statement, 20);

    // =======================================
    // Counts the data
    //
    // Counts the total octets and count
    // =======================================

    do
    {
      CassError rc;
      CassFuture *future = nullptr;

      // Executes the query, and checks for errors
      future = cass_session_execute(cassandra->c_Session, statement);
      cass_future_wait(future);

      rc = cass_future_error_code(future);
      if (rc != CASS_OK)
      {
        std::string message = "cass_session_execute() failed: ";
        message += CassandraConnection::getError(future);

        cass_future_free(future);
        cass_statement_free(statement);
        throw DatabaseException(EXCEPT_DEBUG(message));
      }

      // Creates the iterator and starts looping
      // - over the received data
      const CassResult *result = cass_future_get_result(future);
      CassIterator *iterator = cass_iterator_from_result(result);

      while (cass_iterator_next(iterator))
      {
        const CassRow *row = cass_iterator_get_row(iterator);
        int64_t octets;
        int64_t bucket;
        CassUuid uuid;

        // Gets the values
        cass_value_get_int64(cass_row_get_column_by_name(
          row, "e_size_octets"),
          &octets
        );
        cass_value_get_int64(cass_row_get_column_by_name(
          row, "e_bucket"),
          &bucket
        );
        cass_value_get_uuid(cass_row_get_column_by_name(
          row, "e_email_uuid"),
          &uuid
        );

        // Pushes to the result
        ret.push_back(std::tuple<CassUuid, int64_t, int64_t>(uuid, octets, bucket));
      }

      // Handles the paging stuff
      hasMorePages = cass_result_has_more_pages(result);
      if (hasMorePages)
        cass_statement_set_paging_state(statement, result);

      // Frees the memory
      cass_result_free(result);
      cass_iterator_free(iterator);
      cass_future_free(future);
    } while (hasMorePages);

    // Frees the memory and returns
    cass_statement_free(statement);
    std::reverse(ret.begin(), ret.end());
    return ret;
  }

  /**
   * Delets an email shortcut
   *
   * @Param {CassandraConnection *} cassandra
   * @Param {const std::string &} domain
   * @Param {const CassUuid &} ownersUuid
   * @Param {const CassUuid &} emailUuid
   */
  void EmailShortcut::deleteOne(
    CassandraConnection *cassandra,
    const std::string &domain,
    const CassUuid &ownersUuid,
    const CassUuid &emailUuid
  )
  {
    const char *query = "DELETE FROM fannst.email_shortcuts WHERE e_domain=? AND e_type=? AND e_owners_uuid=? AND e_email_uuid=?";
    CassStatement *statement = nullptr;
    CassFuture *future = nullptr;
    CassError rc;

    // Creates the statement and binds the values
    statement = cass_statement_new(query, 4);
    cass_statement_bind_string(statement, 0, domain.c_str());
    cass_statement_bind_int32(statement, 1, EmailType::ET_INCOMMING);
    cass_statement_bind_uuid(statement, 2, ownersUuid);
    cass_statement_bind_uuid(statement, 3, emailUuid);

    // Executes the query and checks for errors
    future = cass_session_execute(cassandra->c_Session, statement);
    cass_future_wait(future);

    rc = cass_future_error_code(future);
    if (rc != CASS_OK)
    {
      std::string error = "Could not delete email shortcut: ";
      error += CassandraConnection::getError(future);
      cass_future_free(future);
      cass_statement_free(statement);
      throw DatabaseException(error);
    }

    // Frees the memory
    cass_future_free(future);
    cass_statement_free(statement);
  }
}
