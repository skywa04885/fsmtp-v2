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

#include "default.h"

string __ssl_get_error()
{
	BIO *bio = nullptr;
	char *buffer = nullptr;
	size_t bufferLen;

	bio = BIO_new(BIO_s_mem());
	ERR_print_errors(bio);

	bufferLen = BIO_get_mem_data(bio, &buffer);
	string error(buffer, bufferLen);

	BIO_free(bio);
	return error;
}

string __except_debug(const char *file, const size_t line, const string& mess) {
	ostringstream oss;
	oss << file << ": " << line << "->" << mess;
	return oss.str();
}