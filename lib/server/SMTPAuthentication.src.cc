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
	/**
	 * Parses the username and password from the base64 hash
	 *
	 * @Param {const std::string &} hash
	 * @Return {std::string}
	 * @Return {std::string}
	 */
	std::tuple<std::string, std::string> getUserAndPassB64(
		const std::string &hash
	)
	{
		int32_t rc;

		// =================================
		// Decodes the hash
		// 
		// Decodes the base64 hash to 
		// - normal string
		// =================================

		char *decoded = new char[hash.size()];
		BIO *base64 = nullptr, *bio = nullptr;
		BUF_MEM *bufMem = nullptr;

		// Prepares for the decoding, and sets the flags
		base64 = BIO_new(BIO_f_base64());
		bio = BIO_new_mem_buf(hash.c_str(), hash.size());
		bio = BIO_push(base64, bio);
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		// Reads the decoded string
		rc = BIO_read(bio, decoded, hash.size());
		if (rc < 0)
		{
			BIO_free_all(bio);
			delete[] decoded;
			throw std::runtime_error("Could not decode base64");
		}

		// Loops over the decoded value, and searches for the separator
		char *c = decoded;
		c++;
		int32_t i;
		for (i = 0; i < rc; i++)
		{
			if (*c == '\0') break;
			else c++;
		}

		// Parses the username and password into one string,
		// - frees the memory and returns
		std::string username(decoded+1, i), password(c+1, rc-i-2);
		delete[] decoded;
		BIO_free_all(bio);
		return std::tuple(username, password);
	}
}