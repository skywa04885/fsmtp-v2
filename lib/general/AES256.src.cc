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

#include "AES256.src.h"

namespace FSMTP::AES256
{
	static unsigned char _randomDict[] = {
		'a','b','c','d','e','f','g','h','i','j',
		'k','l','m','n','o','p','q','r','s','t',
		'u','v','w','x','y','z','A','B','C','D',
		'E','F','G','H','I','J','K','L','M','N',
		'O','P','Q','R','S','T','U','V','W','X',
		'Y','Z','1','2','3','4','5','6','7','8',
		'9','0'
	};

	std::string encrypt(
		const std::string &raw,
		const std::string &password
	)
	{
		unsigned char iv[_AES256_IV_SIZE_BYTES];
		unsigned char key[_AES256_KEY_SIZE_BYTES];
		unsigned char salt[_AES256_KEY_SALT_SIZE];
		unsigned char ret[(raw.size()/16 + 1) * 16]; // TODO: Make dynamic based on AES256 algo
		EVP_CIPHER_CTX *ctx = nullptr;
		int32_t len, ciptherTextLen, rc;

		// ======================================
		// Prepares
		// 
		// Generates the iv, and creates the key
		// ======================================

		// Creates the random engine and the uniform int
		// - distribution so we can generate the salt and iv
		std::random_device rd;
		std::mt19937 re(rd());
		std::uniform_int_distribution<int> dist(0, sizeof (_randomDict));

		// Generates the IV
		for (std::size_t i = 0; i < _AES256_IV_SIZE_BYTES; i++)
			iv[i] = _randomDict[dist(re)];

		// Generates the salt
		for (std::size_t i = 0; i < _AES256_KEY_SALT_SIZE; i++)
			salt[i] = _randomDict[dist(re)];

		// Generates the 256 bit key
		rc = PKCS5_PBKDF2_HMAC_SHA1(
			password.c_str(),
			password.size(),
			salt,
			_AES256_KEY_SALT_SIZE,
			_AES256_KEY_ITER,
			_AES256_KEY_SIZE_BYTES,
			key
		);
		if (rc < 0)
			throw std::runtime_error("PKCS5_PBKDF2_HMAC_SHA1() failed");

		// ======================================
		// Performs the encryption
		//
		// Performs the AES256 CBC encryption
		// ======================================

		// Create and initialize the context,
		// - if something goes wrong throw an error
		ctx = EVP_CIPHER_CTX_new();
		if (!ctx)
			throw std::runtime_error("EVP_CIPHER_CTX_new() failed");

		// Initializes the encryption operation
		rc = EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);
		if (rc != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			throw std::runtime_error("EVP_EncryptInit_ex() failed");
		}

		// Provides the message that should be encrypted
		rc = EVP_EncryptUpdate(
			ctx, 
			ret, 
			&len, 
			reinterpret_cast<const unsigned char *>(raw.c_str()), 
			raw.size()
		);
		if (rc != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			throw std::runtime_error("EVP_EncryptUpdate() failed");
		}
		ciptherTextLen = len;

		// Gets the encryption result
		rc = EVP_EncryptFinal_ex(ctx, ret + len, &len);
		if (rc != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			throw std::runtime_error("EVP_EncryptFinal_ex() failed");
		}
		ciptherTextLen += len;

		// Frees the memory
		EVP_CIPHER_CTX_free(ctx);

		// ======================================
		// Finishes
		//
		// Turns the ciphertext into base64
		// ======================================

		BIO *base64 = nullptr, *bio = nullptr;
		BUF_MEM *bufMem = nullptr;

		// Initializes the BIO's and sets the flags
		base64 = BIO_new(BIO_f_base64());
		bio = BIO_new(BIO_s_mem());
		bio = BIO_push(base64, bio);
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		// Performs the encoding
		BIO_write(bio, ret, ciptherTextLen);
		BIO_flush(bio);
		BIO_get_mem_ptr(bio, &bufMem);

		// Creates the encrypted string, and frees
		// - the bios
		std::string encrypted(bufMem->data, bufMem->length);
		BIO_free_all(bio);

		// Creates the result, with the iv and salt
		std::string result = encrypted;
		result += '.';
		result += std::string(reinterpret_cast<const char *>(iv), _AES256_IV_SIZE_BYTES);
		result += '.';
		result += std::string(reinterpret_cast<const char *>(salt), _AES256_KEY_SALT_SIZE);

		return result;
	}

	std::string decrypt(
		const std::string &encrypted,
		const std::string &password
	)
	{
		int32_t rc, len, plainTextLen = 0, decodedLen;
		unsigned char iv[_AES256_IV_SIZE_BYTES];
		unsigned char salt[_AES256_KEY_SALT_SIZE];
		unsigned char key[_AES256_KEY_SIZE_BYTES];
		EVP_CIPHER_CTX *ctx = nullptr;

		// ======================================
		// Prepares
		//
		// Gets the iv and salt from the encrypted
		// - text
		// ======================================

		// Gets the indexes
		std::size_t ivIndex = encrypted.find_first_of('.');
		if (ivIndex == std::string::npos)
			throw std::runtime_error("Could not find iv dot");
		std::size_t saltIndex = encrypted.find_last_of('.');
		if (saltIndex == std::string::npos)
			throw std::runtime_error("Could not find salt dot");

		// Gets the IV and salt, and stores them inside of the buffers
		memcpy(iv, &encrypted.c_str()[ivIndex + 1], _AES256_IV_SIZE_BYTES);
		memcpy(salt, &encrypted.c_str()[saltIndex + 1], _AES256_KEY_SALT_SIZE);

		// Creates the key, and throws error if something goes wrong
		rc = PKCS5_PBKDF2_HMAC_SHA1(
			password.c_str(),
			password.size(),
			salt,
			_AES256_KEY_SALT_SIZE,
			_AES256_KEY_ITER,
			_AES256_KEY_SIZE_BYTES,
			key
		);
		if (rc < 0)
			throw std::runtime_error("PKCS5_PBKDF2_HMAC_SHA1() failed");

		// ======================================
		// Decodes
		//
		// Decodes the Base64 string to bytes
		// ======================================

		BIO *base64 = nullptr, *bio = nullptr;
		std::string toDecode = encrypted.substr(0, ivIndex);
		unsigned char *decoded = new unsigned char[toDecode.size()];

		// Prepares the decoder
		base64 = BIO_new(BIO_f_base64());
		bio = BIO_new_mem_buf(toDecode.c_str(), toDecode.size());
		bio = BIO_push(base64, bio);
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		// Reads the decoded base64 data
		decodedLen = BIO_read(bio, decoded, toDecode.size());

		// ======================================
		// Decrypts
		//
		// Performs the decryption
		// ======================================

		unsigned char ret[decodedLen]; // TODO: Make possible with dynamic size

		// Initializes the ctx, and throws error if it goes wrong
		ctx = EVP_CIPHER_CTX_new();
		if (!ctx)
		{
			BIO_free_all(bio);
			delete[] decoded;
			throw std::runtime_error("EVP_CIPHER_CTX_new() failed");
		}

		// Initializes the decryption operation
		rc = EVP_DecryptInit(ctx, EVP_aes_256_cbc(), key, iv);
		if (rc != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			BIO_free_all(bio);
			delete[] decoded;
			throw std::runtime_error("EVP_DecryptInit() failed");
		}

		// Provides the encrypted message
		rc = EVP_DecryptUpdate(
			ctx,
			ret,
			&len,
			decoded,
			decodedLen
		);
		if (rc != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			BIO_free_all(bio);
			delete[] decoded;
			throw std::runtime_error("EVP_DecryptUpdate() failed");
		}
		plainTextLen = len;

		// Gets the decrypted bytes
		rc = EVP_DecryptFinal_ex(ctx, ret + plainTextLen, &len);
		if (rc != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			BIO_free_all(bio);
			delete[] decoded;
			throw std::runtime_error("EVP_DecryptFinal_ex() failed");
		}
		plainTextLen += len;

		// Creates the result, frees the memory and returns
		std::string result(reinterpret_cast<const char *>(ret), plainTextLen);
		EVP_CIPHER_CTX_free(ctx);
		BIO_free_all(bio);
		delete[] decoded;
		return result;
	}
}