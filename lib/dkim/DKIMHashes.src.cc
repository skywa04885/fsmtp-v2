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

#include "DKIMHashes.src.h"

namespace FSMTP::DKIM::Hashes
{
	/**
	 * Creates an SHA256-Base64 Hash using openssl
	 *
	 * @Param {const std::string &} raw
	 * @Return {std::string}
	 */
	std::string sha256base64(const std::string &raw)
	{
		int32_t rc;

		// ==================================
		// Performs the hash
		//
		// Generates the hash from the raw
		// - data
		// ==================================

		// Creates the context and initializes it
		SHA256_CTX ctx;
		rc = SHA256_Init(&ctx);
		if (rc < 0)
			throw std::runtime_error("Could not initialize the SHA256 context");

		// Updats the hash
		rc = SHA256_Update(&ctx, raw.c_str(), raw.size());
		if (rc < 0)
			throw std::runtime_error("Could not update the SHA256 context");

		// Digests the hash
		uint8_t *digest = new uint8_t[SHA256_DIGEST_LENGTH];
		rc = SHA256_Final(digest, &ctx);
		if (rc < 0)
			throw std::runtime_error("Could not digest SHA256 context");

		// ==================================
		// Turn into Base64
		//
		// Turn the generated hash into base
		// - 64, so we can store it inside
		// - the email header
		// ==================================

		BIO *bio = nullptr, *base64 = nullptr;
		BUF_MEM *bufMem = nullptr;

		// Initializes the base64 encoder and sets the flags
		base64 = BIO_new(BIO_f_base64());
		bio = BIO_new(BIO_s_mem());
		bio = BIO_push(base64, bio);
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		// Generatest he base64 encoded string
		BIO_write(bio, digest, SHA256_DIGEST_LENGTH);
		BIO_flush(bio);
		BIO_get_mem_ptr(bio, &bufMem);

		// Creates the result, and frees the memory
		std::string res(bufMem->data, bufMem->length);
		BIO_free_all(bio);
		delete[] digest;
		return res;
	}

	/**
	 * Generates the RSA-SHA256 signature and returns it in
	 * - base64 format
	 *
	 * @param {const std::string &} raw
	 * @Param {const char *} privateKeyFile
	 * @return {std::string}
	 */
	std::string RSASha256generateSignature(
		const std::string &raw, 
		const char *privateKeyFile
	)
	{
		int32_t rc;

		// =====================================
		// Reads the private key file
		//
		// Because the signing process involves
		// - some cryptographic stuff, we need
		// - to read our private dkim file
		// =====================================

		// Creates the file pointer (rt = read text)
		FILE *privateKey = fopen(privateKeyFile, "rt");
		if (!privateKey)
		{
			std::string message = "fopen() failed: ";
			message += strerror(errno);
			throw std::runtime_error(message);
		}

		// Reads the private key
		RSA *rsa = PEM_read_RSAPrivateKey(privateKey, nullptr, nullptr, nullptr);
		if (!rsa)
			throw std::runtime_error("PEM_read_RSAPrivateKey() failed");

		// Closes the file
		fclose(privateKey);

		// =====================================
		// Performs the signing process
		//
		// Signs the specified contents
		// =====================================

		EVP_MD_CTX *rsaSignContext = EVP_MD_CTX_new();
		EVP_PKEY *evpPrivateKey = EVP_PKEY_new();

		// Assigns the RSA Private key to the EVP Private key
		EVP_PKEY_assign_RSA(evpPrivateKey, rsa);

		// Initializes the signer
		rc = EVP_DigestSignInit(
			rsaSignContext,
			nullptr,
			EVP_sha256(),
			nullptr,
			evpPrivateKey
		);
		if (rc < 0)
			throw std::runtime_error("EVP_DigestSignInit() failed");

		// Updates the signer
		rc = EVP_DigestSignUpdate(rsaSignContext, raw.c_str(), raw.size());
		if (rc < 0)
			throw std::runtime_error("EVP_DigestSignUpdate() failed");

		// Gets the signed content length, and allocates the required
		// - memory so we wont have any undefined behaviour
		std::size_t len;
		rc = EVP_DigestSignFinal(rsaSignContext, nullptr, &len);
		if (rc < 0)
			throw std::runtime_error("EVP_DigestSignFinal() failed");
		uint8_t *buffer = new uint8_t[len];

		// Gets the result and stores it inside of the buffer
		// - and frees the memory
		rc = EVP_DigestSignFinal(rsaSignContext, buffer, &len);
		if (rc < 0)
			throw std::runtime_error("EVP_DigestSignUpdate() failed");
		EVP_MD_CTX_free(rsaSignContext);
		EVP_PKEY_free(evpPrivateKey);

		// =====================================
		// Generates the base64 string
		//
		// Gets the raw bytes and turns them
		// - into an base64 string
		// =====================================		

		BIO *bio = nullptr, *base64 = nullptr;
		BUF_MEM *bufMem = nullptr;

		// Initializes the BIO's and sets the flags
		base64 = BIO_new(BIO_f_base64());
		bio = BIO_new(BIO_s_mem());
		bio = BIO_push(base64, bio);
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		// Writes the data to the BIO and encodes it
		// - into base64
		BIO_write(bio, buffer, len);
		BIO_flush(bio);
		BIO_get_mem_ptr(bio, &bufMem);

		// Turns the bio into an result string, and free's the memory
		std::string res(bufMem->data, bufMem->length);
		BIO_free_all(bio);
		delete[] buffer;
		return res;
	}

	/**
	 * Verifies an signature using the public key
	 */
	bool RSASha256verify(const string &signature, const string &raw, const string &pubKey) {
		// Formats the key in a way openssl can read, this is in the
		//  pem format.

		string readyPubKey = "-----BEGIN PUBLIC KEY-----\r\n";

		size_t leftLength = pubKey.length(), usedLength = 0;
		while (leftLength > 0) {
			if (leftLength > 64) {
				readyPubKey += pubKey.substr(usedLength, 64);
				readyPubKey += "\r\n";
				usedLength += 64;
				leftLength -= 64;
			} else {
				readyPubKey += pubKey.substr(usedLength, leftLength);
				readyPubKey += "\r\n";
				usedLength += leftLength;
				leftLength -= leftLength;
			}
		}

		readyPubKey += "-----END PUBLIC KEY-----";

		// =====================================
		// Decodes the Base64 string
		//
		// Due to transmission we encoded the
		//  binary signature to base64, so lets
		//  decode it
		// =====================================

		BIO *base64 = nullptr, *bio = nullptr;
		unsigned char *decodedSignature = new unsigned char[raw.size()];

		// Prepares the decoder
		base64 = BIO_new(BIO_f_base64());
		bio = BIO_new_mem_buf(signature.c_str(), signature.length());
		bio = BIO_push(base64, bio);
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
		DEFER(BIO_free_all(bio));

		// Reads the decoded base64 data
		int32_t decodedLen = BIO_read(bio, decodedSignature, raw.size());

		// =====================================
		// Gets the RSA key
		// =====================================

		BIO *pubKeyBio = BIO_new(BIO_s_mem());
		BIO_write(pubKeyBio, readyPubKey.c_str(), readyPubKey.length());
		DEFER(BIO_free(pubKeyBio));

		cout << readyPubKey << endl;
		RSA *rsa = nullptr;
		if (PEM_read_bio_RSA_PUBKEY(pubKeyBio, &rsa, nullptr, nullptr) == nullptr) {
			throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
		}
		DEFER(RSA_free(rsa));

		// =====================================
		// Validates the signature
		// =====================================

		EVP_MD_CTX *rsaVerifyContext = EVP_MD_CTX_new();
		EVP_PKEY *rsaVerifyKey = EVP_PKEY_new();

		// Assigns the public key to the pkey
		EVP_PKEY_assign_RSA(rsaVerifyKey, rsa);

		// Initializes the signature verifier
		//  with the public key
		if (EVP_DigestVerifyInit(
			rsaVerifyContext, nullptr, EVP_sha256(), 
			nullptr, rsaVerifyKey
		) < 0) throw runtime_error(EXCEPT_DEBUG(string("EVP_DigestVerifyInit() failed:") + SSL_STRERROR)); 
	}
}