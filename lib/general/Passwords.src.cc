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

#include "Passwords.src.h"

namespace FSMTP
{
	static char _saltDict[] = {
		'a','b','c','d','e','f','g','h','i','j',
		'k','l','m','n','o','p','q','r','s','t',
		'u','v','w','x','y','z','A','B','C','D',
		'E','F','G','H','I','J','K','L','M','N',
		'O','P','Q','R','S','T','U','V','W','X',
		'Y','Z','1','2','3','4','5','6','7','8',
		'9','0'
	};

	/**
	 * Verifies an password with existing comparable password
	 *
	 * @Param {const std::string &} password
	 * @Param {const std::string &} comparable
	 * @Return {bool}
	 */
	bool passwordVerify(
		const std::string &password,
		const std::string &compared
	)
	{
		int32_t rc;
		unsigned char out[32];
		char hash[128];

		// Gets the salt from the old hash, and stores it inside
		// - of the salt, if not found throw error, else store
		// - it inside of an string
		std::size_t saltIndex = compared.find_first_of('.');
		if (saltIndex == std::string::npos)
			throw std::runtime_error("Could not derive salt");
		std::string salt = compared.substr(saltIndex + 1);

		// Performs the comparison
		std::string newHash = passwordHashOnly(password, salt);
		if (newHash == compared.substr(0, saltIndex))
			return true;
		else return false;
	}

	/**
	 * Generates salt and hashes an password
	 *
	 * @Param {const std::string &password}
	 * @Return {std::string}
	 */
	std::string passwordHash(const std::string &password)
	{
		std::string salt;

		// Generates the salt, using the C++
		// - random stuff
		std::random_device rd;
		std::mt19937 re(rd());
		std::uniform_int_distribution<int> dist(0, sizeof (_saltDict) - 1);
		for (std::size_t i = 0; i < 25; i++)
			salt += _saltDict[dist(re)];

		// Returns the hash
		std::string res = passwordHashOnly(password, salt);
		res += '.';
		res += salt;
		return res;
	}
	
	/**
	 * Only hashes an password to base64, give it some existing salt
	 *
	 * @Param {const std::string &} password
	 * @Param {const std::string &} salt
	 * @Return {std::string}
	 */
	std::string passwordHashOnly(const std::string &password, const std::string &salt)
	{
		int32_t rc;
		unsigned char out[32];
		char hash[128];
		
		// ====================================
		// Hashes
		//
		// Generates the salt and performs
		// - the hash
		// ====================================

		// Performs the hash
		rc = PKCS5_PBKDF2_HMAC_SHA1(
			password.c_str(),
			password.size(),
			reinterpret_cast<const unsigned char *>(salt.c_str()),
			salt.size(),
			4000,
			sizeof (out),
			out
		);

		// ====================================
		// Encodes
		//
		// Generates the Base64 string
		// ====================================

		BIO *bio = nullptr, *base64 = nullptr;
		BUF_MEM *bufMem = nullptr;

		// Initializes the base64 encoder and sets the flags
		base64 = BIO_new(BIO_f_base64());
		bio = BIO_new(BIO_s_mem());
		bio = BIO_push(base64, bio);
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		// Generates the bsae64 encoded string
		BIO_write(bio, out, sizeof(out));
		BIO_flush(bio);
		BIO_get_mem_ptr(bio, &bufMem);

		// Creates the result and frees the memory
		std::string res(bufMem->data, bufMem->length);
		BIO_free_all(bio);

		// Frees the memory
		return res;
	}
}