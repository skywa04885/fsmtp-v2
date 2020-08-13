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

#include "SMTPAuthentication.src.h"

namespace FSMTP::Server
{
	tuple<string, string> getUserAndPassB64(const string &hash) {
		auto &conf = Global::getConfig();
		char *decoded = new char[hash.size()];
		BIO *base64 = nullptr, *bio = nullptr;
		BUF_MEM *bufMem = nullptr;
		int32_t rc;

		// Decodes the base64 string, this will be done with openssl
		//  since it is faster than the one i wrote. If this fails
		//  we free the memory and throw an runtime_error.

		base64 = BIO_new(BIO_f_base64());
		bio = BIO_new_mem_buf(hash.c_str(), hash.size());
		bio = BIO_push(base64, bio);
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		rc = BIO_read(bio, decoded, hash.size());
		if (rc < 0) {
			BIO_free_all(bio);
			delete[] decoded;
			throw runtime_error("Could not decode base64");
		}

		// Finds the null termination char, this will separate the password from
		//  the username, we cannot use str.find() since this will assume that the
		//  null terminator is the end of the string.

		char *c = decoded;
		c++;
		int32_t i;
		for (i = 0; i < rc; i++)
		{
			if (*c == '\0') break;
			else c++;
		}
		if (i == 0) {
			throw runtime_error("Could not find separator");
		}

		// Parses the username and password into one string,
		// - frees the memory and returns
		string username(decoded+1, i), password(c+1, rc-i-2);

		// Adds the default domain if not specified
		if (username.find_first_of('@') == string::npos)
		{
			username += '@';
			username += conf["domain"].asCString();
		}
		delete[] decoded;
		BIO_free_all(bio);
		return tuple<string, string>(username, password);
	}

	bool authVerify(
		RedisConnection *redis,
		CassandraConnection *cassandra,
		const std::string &user,
		const std::string &password,
		AccountShortcut &shortcutTarget
	) {
		EmailAddress address(user);	
		LocalDomain domain;

		// Checks if the domain is local, if this is the case we
		//  proceed with finding the account shortcut, if one of these
		//  fails we know that the user is not local, and throw error

		try {
			domain = LocalDomain::findRedis(address.getDomain(), redis);
		} catch (const EmptyQuery &e) {
			throw runtime_error("Domain not local");
		}

		try {
			shortcutTarget = AccountShortcut::findRedis(
				redis, address.getDomain(), address.getUsername()
			);
		} catch (const EmptyQuery &e) {
			throw runtime_error("User not found on server");
		}

		// Since the user exists, get his public key and password
		std::string uPass, uPubKey;
		try {
			std::tie(uPass, uPubKey) = Account::getPassAndPublicKey(
				cassandra, 
				shortcutTarget.a_Domain, 
				shortcutTarget.a_Bucket, 
				shortcutTarget.a_UUID
			);
		} catch (const EmptyQuery &e) {
			std::string message = "User not found in database, possible removed: ";
			message += e.what();
			throw std::runtime_error(message);
		}

		return passwordVerify(password, uPass);
	}
}