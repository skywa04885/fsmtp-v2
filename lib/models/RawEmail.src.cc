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

using namespace FSMTP::Models;

RawEmail::RawEmail() {}

RawEmail::RawEmail(
  int64_t bucket, const string &domain, const CassUuid &owner,
  const CassUuid &email, const string &content
):
  e_Bucket(bucket), e_Domain(domain), e_OwnersUUID(owner),
  e_EmailUUID(email), e_Content(content)
{}

void RawEmail::save(CassandraConnection *cassandra) {
  const char *query = R"(INSERT INTO fannst.raw_emails (
    e_domain, e_bucket, e_owners_uuid,
    e_email_uuid, e_content
  ) VALUES (
    ?, ?, ?,
    ?, ?
  ))";
  CassFuture *future = nullptr;
  CassStatement *statement = nullptr;

  // Prepares the statement and binds the value, after which we execute
  //  and check if any errors have occured

  statement = cass_statement_new(query, 5);
  DEFER(cass_statement_free(statement));
  cass_statement_bind_string(statement, 0, this->e_Domain.c_str());
  cass_statement_bind_int64(statement, 1, this->e_Bucket);
  cass_statement_bind_uuid(statement, 2, this->e_OwnersUUID);
  cass_statement_bind_uuid(statement, 3, this->e_EmailUUID);
  cass_statement_bind_string(statement, 4, this->e_Content.c_str());

  future = cass_session_execute(cassandra->c_Session, statement);
  DEFER(cass_future_free(future));
  cass_future_wait(future);

  if (cass_future_error_code(future) != CASS_OK) {
    string error = "cass_session_execute() failed: ";
    error += CassandraConnection::getError(future);
    throw DatabaseException(EXCEPT_DEBUG(error));
  }
}

RawEmail RawEmail::get(
  CassandraConnection *cassandra,
  const string &domain,
  const CassUuid &ownersUuid,
  const CassUuid &emailUuid,
  const int64_t bucket
) {
  const char *query = "SELECT e_content FROM fannst.raw_emails WHERE e_bucket=? AND e_domain=? AND e_owners_uuid=? AND e_email_uuid=?";
  CassStatement *statement = nullptr;
  CassFuture *future = nullptr;
  const char *content = nullptr;
  size_t contentLen;
  RawEmail ret;

  // Prepares the statement and executes, if any error may occur
  //  we will throw an database exception

  statement = cass_statement_new(query, 4);
  DEFER(cass_statement_free(statement));
  cass_statement_bind_int64(statement, 0, bucket);
  cass_statement_bind_string(statement, 1, domain.c_str());
  cass_statement_bind_uuid(statement, 2, ownersUuid);
  cass_statement_bind_uuid(statement, 3, emailUuid);

  future = cass_session_execute(cassandra->c_Session, statement);
  DEFER(cass_future_free(future));
  cass_future_wait(future);

  if (cass_future_error_code(future) != CASS_OK) {
    string error = "cass_session_execute() failed: ";
    error += CassandraConnection::getError(future);
    throw DatabaseException(EXCEPT_DEBUG(error)); 
  }

  // Gets the raw emails content, if there is no raw email we throw
  //  an empty query to inform the caller

  const CassResult *result = cass_future_get_result(future);
  DEFER(cass_result_free(result));
  const CassRow *row = cass_result_first_row(result);
  
  if (!row) {
    throw EmptyQuery("Could not find raw email");
  }

  cass_value_get_string(cass_row_get_column_by_name(row, "e_content"), &content, &contentLen);

  ret.e_Content.append(content, contentLen);
  ret.e_Domain = domain;
  ret.e_OwnersUUID = ownersUuid;
  ret.e_EmailUUID = emailUuid;
  ret.e_Bucket = bucket;

  return ret;
}

void RawEmail::deleteOne(
  CassandraConnection *cassandra,
  const string &domain,
  const CassUuid &ownersUuid,
  const CassUuid &emailUuid,
  const int64_t bucket
) {
  const char *query = "DELETE FROM fannst.raw_emails WHERE e_bucket=? AND e_domain=? AND e_owners_uuid=? AND e_email_uuid=?";
  CassStatement *statement = nullptr;
  CassFuture *future = nullptr;

  // Prepares the statement, binds the values and performs
  //  the delete operation, after which we check if anything went wrong

  statement = cass_statement_new(query, 4);
  DEFER(cass_statement_free(statement));
  cass_statement_bind_int64(statement, 0, bucket);
  cass_statement_bind_string(statement, 1, domain.c_str());
  cass_statement_bind_uuid(statement, 2, ownersUuid);
  cass_statement_bind_uuid(statement, 3, emailUuid);

  future = cass_session_execute(cassandra->c_Session, statement);
  DEFER(cass_future_free(future));
  cass_future_wait(future);

  if (cass_future_error_code(future) != CASS_OK) {
    string error = "Could not delete raw email: ";
    error += CassandraConnection::getError(future);
    throw DatabaseException(error);
  }
}
