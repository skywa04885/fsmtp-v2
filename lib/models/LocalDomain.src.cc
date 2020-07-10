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

#include "LocalDomain.src.h"

namespace FSMTP::Models
{
	/**
	 * Creates an new local domain and generates
	 * the uuid automatically for it
	 * 
	 * @Param {std::string} l_Domain
	 * @Return void
	 */
	LocalDomain::LocalDomain(const std::string &l_Domain):
		l_Domain(l_Domain)
	{
		// Creates the UUID gen and generates an new 
		// - user id for the domain
		CassUuidGen *uuidGen = cass_uuid_gen_new();
		cass_uuid_gen_time(uuidGen, &this->l_UUID);
		cass_uuid_gen_free(uuidGen);
	}

	/**
	 * Empty constructor, will just initialize the values

	 * @Param void
	 * @Return void
	 */
	LocalDomain::LocalDomain(void):
		l_Domain()
	{}

	/**
	 * Searches in the database for an domain with that
	 * specific ID
	 *
	 * @Param {const std::string &} l_Domain
	 * @Param {std::unique_ptr<CassandraConnection> &} database
	 * @Return void
	 */
	void LocalDomain::getByDomain(
		const std::string &l_Domain,
		std::unique_ptr<CassandraConnection>& database
	)
	{
		CassStatement *queryStatement = nullptr;
		CassFuture *queryFuture = nullptr;
		CassError rc;

		// Creates the query and binds the parameters,
		// - after this we execute the query and wait
		// - for results
		const char *query = R"(SELECT e_domain_uuid FROM fannst.local_domain WHERE e_domain = ?)";
		
		queryStatement = cass_statement_new(query, 1);
		cass_statement_bind_string(queryStatement, 0, l_Domain.c_str());

		queryFuture = cass_session_execute(database->c_Session, queryStatement);
		cass_future_wait(queryFuture);

		// Checks if the query was successfully
		// - if not throws error
		rc = cass_future_error_code(queryFuture);
		if (rc != CASS_OK)
		{
			// Gets the error message and combines it
			// - with our own message, then throws it
			const char *err = nullptr;
			std::size_t errLen;
			cass_future_error_message(queryFuture, &err, &errLen);

			std::string errString(err, errLen);
			std::string message = "cass_session_execute() failed: ";
			message += errString;

			throw DatabaseException(message);
		}

		// Gets the data from the query, and stores it in
		// - the current instance
		const CassResult *result = cass_future_get_result(queryFuture);
		const CassRow *row = cass_result_first_row(result);

		if (!row)
		{
			throw EmptyQuery("No LocalDomain found with this specific domain");
		}

		cass_value_get_uuid(cass_row_get_column_by_name(row, "u_uuid"), &this->l_UUID);

		// Frees the memory since the query was already
		// - performed, and we don't want to leak anything
		cass_statement_free(queryStatement);
		cass_future_free(queryFuture);
		cass_result_free(result);
	}
}