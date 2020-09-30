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
	Account::Account():
		a_Bucket(0), a_BirthDate(0),
		a_CreationDate(0), a_Gas(0.0f),
		a_Type(0), a_Flags(0x0), a_StorageUsedInBytes(0),
		a_StorageMaxInBytes(0)
	{}

	int64_t Account::getBucket() {
		int64_t now = chrono::duration_cast<chrono::milliseconds>(
			chrono::high_resolution_clock::now().time_since_epoch()
		).count();
		return now / 1000 / 1000 / 10;
	}

	void Account::generateUUID() {
		CassUuidGen *uuidGen = cass_uuid_gen_new();
		cass_uuid_gen_time(uuidGen, &this->a_UUID);
		cass_uuid_gen_free(uuidGen);
	}

	tuple<string, string> Account::getPassAndPublicKey(
		CassandraConnection *client,
		const string &domain,
		const int64_t bucket,
		const CassUuid &uuid
	) {
		const char *query = R"(SELECT a_password, a_rsa_public 
		FROM fannst.accounts WHERE a_bucket=? AND a_domain=? AND a_uuid=?)";
		CassFuture *future = nullptr;
		CassStatement *statement = nullptr;
		
		// Creates and binds the values to the statement, after which we execute
		//  and check for errors

		statement = cass_statement_new(query, 3);
		DEFER(cass_statement_free(statement));
		cass_statement_bind_int64(statement, 0, bucket);
		cass_statement_bind_string(statement, 1, domain.c_str());
		cass_statement_bind_uuid(statement, 2, uuid);

		future = cass_session_execute(client->c_Session, statement);
		DEFER(cass_future_free(future));
		cass_future_wait(future);

		if (cass_future_error_code(future) != CASS_OK) {
			string message = "cass_session_execute() failed: ";
			message += CassandraConnection::getError(future);
			throw DatabaseException(message);
		}

		// Gets the results from the query, and throw an error if the
		//  query did not return anything

		const CassResult *result = cass_future_get_result(future);
		DEFER(cass_result_free(result));
		const CassRow *row = cass_result_first_row(result);

		if (!row) {
			throw EmptyQuery("Could not find user");
		}

		const char *password = nullptr, *publicKey = nullptr;
		size_t passwordSize, publicKeySize;

		cass_value_get_string(cass_row_get_column_by_name(row, "a_password"), &password, &passwordSize);
		cass_value_get_string(cass_row_get_column_by_name(row, "a_rsa_public"), &publicKey, &publicKeySize);

		string passwordRes(password, passwordSize);
		string pubKeyRes(publicKey, publicKeySize);
		return tuple<string, string>(passwordRes, pubKeyRes);
	}

	void Account::generateKeypair() {
		RSA *rsa = nullptr;
		BIO *bpPublic = nullptr, *bpPrivate = nullptr;
		BUF_MEM *bpPublicMem = nullptr, *bpPrivateMem = nullptr;
		BIGNUM *bne = nullptr;

		// ============================================
		// Generates the RSA keypair
		// ============================================

		bne = BN_new();
		DEFER(BN_free(bne));
		if (BN_set_word(bne, RSA_F4) < 0) {
			throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
		}

		rsa = RSA_new();
		DEFER(RSA_free(rsa));
		if (RSA_generate_key_ex(rsa, 2048, bne, nullptr) < 0) {
			throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
		}

		// ============================================
		// Gets the public key
		// ============================================

		bpPublic = BIO_new(BIO_s_mem());
		DEFER(BIO_free_all(bpPublic));

		if (PEM_write_bio_RSAPublicKey(bpPublic, rsa) < 0) {
			throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
		}

		// Gets the raw memory pointer so we can read the data
		//  and the length from it
		BIO_get_mem_ptr(bpPublic, &bpPublicMem);
		this->a_RSAPublic = string(bpPublicMem->data, bpPublicMem->length);

		// ============================================
		// Gets the private key
		// ============================================

		bpPrivate = BIO_new(BIO_s_mem());
		DEFER(BIO_free_all(bpPrivate));

		if (PEM_write_bio_RSAPrivateKey(
			bpPrivate, rsa, nullptr,
			nullptr, 0, nullptr,
			nullptr
		) < 0) {
			throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
		}

		BIO_get_mem_ptr(bpPrivate, &bpPrivateMem);
		this->a_RSAPrivate = string(bpPrivateMem->data, bpPrivateMem->length);
	}

	void Account::save(CassandraConnection *cassandra) {
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
		CassFuture *future = nullptr;
		CassStatement *statement = nullptr;

		// Prepares the statement and binds the values, after this we perform
		//  the execution and throw an error if this fails

		statement = cass_statement_new(query, 21);
		DEFER(cass_statement_free(statement));
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

		future = cass_session_execute(cassandra->c_Session, statement);
		DEFER(cass_future_free(future));
		cass_future_wait(future);

		if (cass_future_error_code(future) != CASS_OK) {
			string message = "cass_session_execute() failed: ";
			message += CassandraConnection::getError(future);
			throw DatabaseException(message);
		}
	}

	// ======================================
	// The account shortcut stuff
	// ======================================

	AccountShortcut::AccountShortcut(
		int64_t a_Bucket,
		const string &a_Domain,
		const string &a_Username,
		const CassUuid &a_UUID
	):
		a_Bucket(a_Bucket),
		a_Domain(a_Domain),
		a_Username(a_Username),
		a_UUID(a_UUID)
	{}

	AccountShortcut::AccountShortcut() {}

	void AccountShortcut::getPrefix(const string &domain, const string &username, char *buffer) {
		sprintf(buffer, "acc:%s@%s", username.c_str(), domain.c_str());
	}

	void AccountShortcut::save(CassandraConnection *conn) {
		const char *query = R"(INSERT INTO fannst.account_shortcuts (
			a_bucket, a_domain, a_uuid,
			a_username
		) VALUES (
			?, ?, ?,
			?
		))";
		CassFuture *future = nullptr;
		CassStatement *statement = nullptr;

		// Prepares the statement, binds the values and executes the query,
		//  if it fails we throw an exception

		statement = cass_statement_new(query, 4);
		DEFER(cass_statement_free(statement));
		cass_statement_bind_int64(statement, 0, this->a_Bucket);
		cass_statement_bind_string(statement, 1, this->a_Domain.c_str());
		cass_statement_bind_uuid(statement, 2, this->a_UUID);
		cass_statement_bind_string(statement, 3, this->a_Username.c_str());

		future = cass_session_execute(conn->c_Session, statement);
		DEFER(cass_future_free(future));
		cass_future_wait(future);

		if (cass_future_error_code(future) != CASS_OK) {
			string message = "cass_session_execute() failed: ";
			message += CassandraConnection::getError(future);
			throw DatabaseException(message);
		}
	}

	AccountShortcut AccountShortcut::find(
		CassandraConnection *cass, RedisConnection *redis,
		const string &domain, const string &username
	) {
		AccountShortcut res;

		try {
			res = findRedis(redis, domain, username);
		} catch (const EmptyQuery &e) {
			res = findCassandra(cass, domain, username);
			res.saveRedis(redis);
		}

		return res;
	}

	AccountShortcut AccountShortcut::findCassandra(
		CassandraConnection *conn,
		const string &domain,
		const string &username
	) {
		const char *query = R"(SELECT a_bucket, a_uuid 
		FROM fannst.account_shortcuts WHERE a_domain=? AND a_username=?)";
		AccountShortcut res;
		CassStatement *statement = nullptr;
		CassFuture *future = nullptr;

		// Prepares the statement, binds the values and executes it
		//  if it returns null we will throw empty query, and if an
		//  error occurs, we throw an exception.

		statement = cass_statement_new(query, 2);
		DEFER(cass_statement_free(statement));
		cass_statement_bind_string(statement, 0, domain.c_str());
		cass_statement_bind_string(statement, 1, username.c_str());

		future = cass_session_execute(conn->c_Session, statement);
		DEFER(cass_future_free(future));
		cass_future_wait(future);

		if (cass_future_error_code(future) != CASS_OK) {
			string message = "cass_session_execute() failed: ";
			message += CassandraConnection::getError(future);
			throw DatabaseException(message);
		}

		// Gets the results from the query, and throws an error if they
		//  could not be found

		const CassResult *result = cass_future_get_result(future);
		DEFER(cass_result_free(result));
		const CassRow *row = cass_result_first_row(result);

		if (!row) {
			throw EmptyQuery("Could not find AccountShortcut in database");
		}

		cass_value_get_int64(cass_row_get_column_by_name(row, "a_bucket"), &res.a_Bucket);
		cass_value_get_uuid(cass_row_get_column_by_name(row, "a_uuid"), &res.a_UUID);
		res.a_Domain = domain;
		res.a_Username = username;

		return res;
	}

	AccountShortcut AccountShortcut::findRedis(
		RedisConnection *redis,
		const string &domain,
		const string &username
	) {
		AccountShortcut res;

		char prefix[512], command[612];
		AccountShortcut::getPrefix(domain, username, prefix);
		sprintf(command, "HGETALL %s", prefix);

		// Performs the redis query, and checks if the response types
		//  are as we excepted, if they do not match we throw an error

		redisReply *reply = reinterpret_cast<redisReply *>(redisCommand(
			redis->r_Session, command
		));
		DEFER(freeReplyObject(reply));

		if (reply->type == REDIS_REPLY_NIL || reply->elements <= 0) {
			throw EmptyQuery("Could not find user in Redis");
		} else if (reply->type != REDIS_REPLY_ARRAY) {
			throw DatabaseException("Expected type map, got something else");
		}

		if (reply->element[1]->type != REDIS_REPLY_STRING) {
			throw DatabaseException("Invalid value at position 1, expected number");
		} else res.a_Bucket = stoi(reply->element[1]->str);

		if (reply->element[3]->type != REDIS_REPLY_STRING) {
			throw DatabaseException("Invalid value at position 3: expected string");
		} cass_uuid_from_string(reply->element[3]->str, &res.a_UUID);

		res.a_Domain = domain;
		res.a_Username = username;

		return res;
	}

  void AccountShortcut::saveRedis(RedisConnection *redis) {
		char prefix[512], command[1024];
		AccountShortcut::getPrefix(this->a_Domain, this->a_Username, prefix);

    char uuidBuffer[CASS_UUID_STRING_LENGTH];
    cass_uuid_string(this->a_UUID, uuidBuffer);

		string bucket = to_string(this->a_Bucket);
		sprintf(command, "HMSET %s v1 %s v2 %s", prefix, bucket.c_str(), uuidBuffer);

		// Performs the redis command and checks for any errors
		//  if not return without any error

    redisReply *reply = reinterpret_cast<redisReply *>(
      redisCommand(redis->r_Session, command)
    );
		DEFER(freeReplyObject(reply));

    if (reply->type == REDIS_REPLY_ERROR) {
      string message = "redisCommand() failed: ";
      message += string(reply->str, reply->len);
      throw DatabaseException(message);
    }
  }

	int64_t AccountShortcut::getBucket() const {
		return this->a_Bucket;
	}

	const string &AccountShortcut::getDomain() const {
		return this->a_Domain;
	}

	const string &AccountShortcut::getUsername() const {
		return this->a_Username;
	}

	const CassUuid &AccountShortcut::getUUID() const {
		return this->a_UUID;
	}
}