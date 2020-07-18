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

#include "RawEmail.src.h"

namespace FSMTP::Models
{
  /**
   * Default construtor for the raw email
   *
   * @Param {void}
   * @Return {void}
   */
  RawEmail::RawEmail(void)
  {}

  /**
   * Stores an rawEmail into the database
   *
   * @Param {CassandraConnection *} cassandra
   * @Return {void}
   */
  void RawEmail::save(CassandraConnection *cassandra)
  {
    const char *query = R"(INSERT INTO fannst.raw_emails (
      e_domain, e_bucket, e_owners_uuid,
      e_email_uuid, e_content
    ) VALUES (
      ?, ?, ?,
      ?, ?
    ))";
    CassFuture *future = nullptr;
    CassStatement *statement = nullptr;
    CassError rc;

    // Creates the statement with the query, and binds the values
    statement = cass_statement_new(query, 5);
    cass_statement_bind_string(statement, 0, this->e_Domain.c_str());
    cass_statement_bind_int64(statement, 1, this->e_Bucket);
    cass_statement_bind_uuid(statement, 2, this->e_OwnersUUID);
    cass_statement_bind_uuid(statement, 3, this->e_EmailUUID);
    cass_statement_bind_string(statement, 4, this->e_Content.c_str());

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
}
