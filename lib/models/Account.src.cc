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

#include "Account.src.h"

namespace FSMTP::Models
{
	/**
	 * Default empty constructor for the account
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	Account::Account(void):
		a_Bucket(0), a_BirthDate(0),
		a_CreationDate(0), a_Gas(0.0f),
		a_Type(0), a_Flags(0x0), a_StorageUsedInBytes(0),
		a_StorageMaxInBytes(0)
	{}

	/**
	 * Gets the current user bucket
	 *
	 * @Param {void}
	 * @Return {int64_t}
	 */
	int64_t Account::getBucket(void)
	{
		int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
		return now / 1000 / 1000 / 10;
	}

	/**
	 * Generates an RSA keypair for the account
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void Account::generateKeypair(void)
	{
		int rc;
		RSA *rsa = nullptr;
		BIO *bpPublic = nullptr, *bpPrivate = nullptr;
		BUF_MEM *bpPublicMem = nullptr, *bpPrivateMem = nullptr;
		BIGNUM *bne = nullptr;

		// Generates the RSA key, and to be honest
		// - i do not know WTF is going on here
		bne = BN_new();
		rc = BN_set_word(bne, RSA_F4);
		if (rc < 0)
		{
			BN_free(bne);
			throw std::runtime_error("BN_set_word() failed");
		}

		rsa = RSA_new();
		rc = RSA_generate_key_ex(rsa, 2048, bne, nullptr);
		if (rc < 0)
		{
			BN_free(bne);
			RSA_free(rsa);
			throw std::runtime_error("RSA_generate_key_ex() failed");
		}

		// ============================================
		// Gets the public key
		//
		// Stores the public key in the current class
		// ============================================

		bpPublic = BIO_new(BIO_s_mem());
		
		// Writes the public key into the buffer
		// - if something goes wrong free mem and throw error
		rc = PEM_write_bio_RSAPublicKey(bpPublic, rsa);
		if (rc < 0)
		{
			BN_free(bne);
			RSA_free(rsa);
			BIO_free_all(bpPublic);
			throw std::runtime_error("PEM_write_bio_RSAPublicKey() failed");
		}

		// Gets the memory pointer and stores an copy of the
		// - key in the current class
		BIO_get_mem_ptr(bpPublic, &bpPublicMem);
		this->a_RSAPublic = std::string(bpPublicMem->data, bpPublicMem->length);

		// ============================================
		// Gets the private key
		//
		// Stores the private key in the current class
		// ============================================

		bpPrivate = BIO_new(BIO_s_mem());

		// Writes the private key into the buffer
		// - if something goes wrong free memory and throw error
		rc = PEM_write_bio_RSAPrivateKey(
			bpPrivate,
			rsa,
			nullptr,
			nullptr,
			0,
			nullptr,
			nullptr
		);
		if (rc < 0)
		{
			BN_free(bne);
			RSA_free(rsa);
			BIO_free_all(bpPublic);
			BIO_free_all(bpPrivate);
			throw std::runtime_error("PEM_write_bio_RSAPrivateKey() failed");
		}

		// Gets the memory pointer and stores an copy of the
		// - key in the current class
		BIO_get_mem_ptr(bpPrivate, &bpPrivateMem);
		this->a_RSAPrivate = std::string(bpPrivateMem->data, bpPrivateMem->length);

		// Frees the memory and returns
		BN_free(bne);
		RSA_free(rsa);
		BIO_free_all(bpPublic);
		BIO_free_all(bpPrivate);
	}

	/**
	 * Saves an account in the cassandra database
	 *
	 * @Param {CassadraConnection *} cassandra
	 * @Return {void}
	 */
	void Account::save(CassandraConnection *cassandra)
	{
		CassFuture *future = nullptr;
		CassError rc;
		CassStatement *statement = nullptr;
		const char *query = R"(INSERT INTO fannst.accounts (
			a_username, a_picture_uri, a_password,
			a_domain, a_bucket, a_full_name,
			a_birth_date, a_creation_date, a_rsa_public,
			a_rsa_private, a_gas, a_country, 
			a_region, a_city, a_address, 
			a_phone, a_type, a_uuid,
			a_flags, a_storage_used_bytes, a_storage_max_bytes
		) VALUES (
			?, ?, ?,
			?, ?, ?,
			?, ?, ?,
			?, ?, ?,
			?, ?, ?,
			?, ?, ?,
			?, ?, ?
		))";

		// Creates the statement and binds the values
		statement = cass_statement_new(query, 21);
		cass_statement_bind_string(statement, 0, this->a_Username.c_str());
		cass_statement_bind_string(statement, 1, this->a_PictureURI.c_str());
		cass_statement_bind_string(statement, 2, this->a_Password.c_str());
		cass_statement_bind_string(statement, 3, this->a_Domain.c_str());
		cass_statement_bind_int64(statement, 4, this->a_Bucket);
		cass_statement_bind_string(statement, 5, this->a_FullName.c_str());
		cass_statement_bind_int64(statement, 6, this->a_BirthDate);
		cass_statement_bind_int64(statement, 7, this->a_CreationDate);
		cass_statement_bind_string(statement, 8, this->a_RSAPublic.c_str());
		cass_statement_bind_string(statement, 9, this->a_RSAPrivate.c_str());
		cass_statement_bind_double(statement, 10, this->a_Gas);
		cass_statement_bind_string(statement, 11, this->a_Country.c_str());
		cass_statement_bind_string(statement, 12, this->a_Region.c_str());
		cass_statement_bind_string(statement, 13, this->a_City.c_str());
		cass_statement_bind_string(statement, 14, this->a_Address.c_str());
		cass_statement_bind_string(statement, 15, this->a_Phone.c_str());
		cass_statement_bind_int8(statement, 16, this->a_Type);
		cass_statement_bind_uuid(statement, 17, this->a_UUID);
		cass_statement_bind_int64(statement, 18, this->a_Flags);
		cass_statement_bind_int64(statement, 19, this->a_StorageUsedInBytes);
		cass_statement_bind_int64(statement, 20, this->a_StorageMaxInBytes);

		// Executes the query, waits for it and then
		// - checks if any error occured
		future = cass_session_execute(cassandra->c_Session, statement);
		cass_future_wait(future);

		rc = cass_future_error_code(future);
		if (rc != CASS_OK)
		{
			// Builds the error message, and frees the memory
			std::string message = "cass_session_execute() failed: ";
			message += CassandraConnection::getError(future);
			cass_future_free(future);
			cass_statement_free(statement);
			throw DatabaseException(message);
		}

		// Frees the memory and returns from the storage function
		cass_future_free(future);
		cass_statement_free(statement);
	}

	/**
	 * Default constructor for the AccountShortcut
	 *
	 * @Param {int64_t} a_Bucket
	 * @Param {std::string &} a_Domain
	 * @Param {std::strin &} a_Username
	 * @Param {const CassUuid &} a_Uuid 
	 * @Return {void}
	 */
	AccountShortcut::AccountShortcut(
		int64_t a_Bucket,
		const std::string &a_Domain,
		const std::string &a_Username,
		const CassUuid &a_UUID
	):
		a_Bucket(a_Bucket),
		a_Domain(a_Domain),
		a_Username(a_Username),
		a_UUID(a_UUID)
	{}

	/**
	 * Empty constructor for the account shortcut
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	AccountShortcut::AccountShortcut()
	{}

	/**
	 * Finds an account shortcut in the cassandra
	 * - database
	 *
	 * @Param {std::unique_ptr<CassandraConnection> *} conn
	 * @Param {const std::string &} domain
	 * @Param {const std::string &} username
	 * @Return {AccountShortuct}
	 */
	AccountShortcut AccountShortcut::find(
		CassandraConnection *conn,
		const std::string &domain,
		const std::string &username
	)
	{
		AccountShortcut res;
		CassError rc;
		CassStatement *statement = nullptr;
		CassFuture *future = nullptr;

		// ========================================
  	// Creates the query and executes it
  	//
  	// Creates the statement and adds the final
  	// - variables, then we execute
  	// ========================================

		// Creates the query
		const char *query = R"(SELECT a_bucket, a_uuid 
		FROM fannst.account_shortcuts WHERE a_domain=? AND a_username=?)";

		// Prepares the statement and binds the
		// - values, such as domain and username
		statement = cass_statement_new(query, 2);
		cass_statement_bind_string(statement, 0, domain.c_str());
		cass_statement_bind_string(statement, 1, username.c_str());

		// Executes the query, and waits
		// - for the query to finish
		future = cass_session_execute(conn->c_Session, statement);
		cass_future_wait(future);

		// Checks if the query was successfull
		rc = cass_future_error_code(future);
		if (rc != CASS_OK)
		{
			std::string message = "cass_session_execute() failed: ";
			message += CassandraConnection::getError(future);

			throw DatabaseException(message);
		}

		// Checks if there is any data, and
		// - if so get it
		const CassResult *result = cass_future_get_result(future);
		const CassRow *row = cass_result_first_row(result);

		if (!row)
		{
			cass_result_free(result);
			cass_future_free(future);
			cass_statement_free(statement);
			
			throw EmptyQuery("Could not find AccountShortcut in database");
		}

		cass_value_get_int64(
			cass_row_get_column_by_name(row, "a_bucket"),
			&res.a_Bucket
		);
		cass_value_get_uuid(
			cass_row_get_column_by_name(row, "a_uuid"),
			&res.a_UUID
		);
		res.a_Domain = domain;

		// Frees the memory, and returns
		// - the result
		cass_result_free(result);
		cass_future_free(future);
		cass_statement_free(statement);
		return res;
	}

	/**
	 * Finds an user in the redis server
	 *
	 * @Param {RedisConnection *} redis
	 * @Param {std::string &} domain
	 * @Param {std::string &} username
	 * @Return {AccountShortcut}
	 */
	AccountShortcut AccountShortcut::findRedis(
		RedisConnection *redis,
		const std::string &domain,
		const std::string &username
	)
	{
		AccountShortcut res;

		// Prepares the command
		std::string command = "HGETALL acc:";
		command += username;
		command += '@';
		command += domain;

		// Performs the command
		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command.c_str()
		));

		if (reply->type == REDIS_REPLY_NIL || reply->elements <= 0)
		{
			freeReplyObject(reply);
			throw EmptyQuery("Could not find user in Redis");
		} else if (reply->type != REDIS_REPLY_ARRAY)
		{
			freeReplyObject(reply);
			throw DatabaseException("Expected type map, got something else");
		}

		if (reply->element[1]->type != REDIS_REPLY_STRING)
		{
			freeReplyObject(reply);
			throw DatabaseException("Invalid value at position 1, expected number");
		} else res.a_Bucket = std::stoi(reply->element[1]->str);

		if (reply->element[3]->type != REDIS_REPLY_STRING)
		{
			freeReplyObject(reply);
			throw DatabaseException("Invalid value at position 3: expected string");
		} cass_uuid_from_string(reply->element[3]->str, &res.a_UUID);

		freeReplyObject(reply);
		return res;
	}
}