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

#pragma once

#include <iostream>

// Uncomment in production since it will make you 
// - crazy when the console is being spammed with
// - all kinds of debug crap
#define _SMTP_DEBUG

#ifdef _SMTP_DEBUG
#define DEBUG_ONLY(A) A
#else
#define DEBUG_ONLY(A)
#endif

#ifdef _SMTP_DEBUG
#define DEBUG_PRINT(A, B) A << DEBUG << B << ENDL << CLASSIC
#else
#define DEBUG_PRINT(A, B)
#endif

#define BINARY_COMPARE(A, B) (A & B) == B

#define FATAL_ERROR(A) std::cerr << "\033[32m" << __LINE__ << '@' << __FILE__ << "\033[31m" << ": " << A << "\033[0m" << std::endl

#ifndef _SMTP_SERVICE_DOMAIN
#define _SMTP_SERVICE_DOMAIN "mail.fannst.nl"
#endif

#ifndef _SMTP_SERVICE_DKIM_DOMAIN
#define _SMTP_SERVICE_DKIM_DOMAIN "fannst.nl"
#endif

#ifndef _SMTP_SERVICE_NODE_NAME
#define _SMTP_SERVICE_NODE_NAME "LUKERIEFF_MCLUST_A001"
#endif

#ifndef _SMTP_RECEIVE_BUFFER_SIZE
#define _SMTP_RECEIVE_BUFFER_SIZE 32
#endif

#ifndef _SMTP_SSL_CERT_PATH
#define _SMTP_SSL_CERT_PATH "../env/keys/cert.pem"
#endif

#ifndef _SMTP_SSL_KEY_PATH
#define _SMTP_SSL_KEY_PATH "../env/keys/key.pem"
#endif

#ifndef _SMTP_SSL_PASSPHRASE_PATH
#define _SMTP_SSL_PASSPHRASE_PATH "../env/keys/pass.txt"
#endif

#ifndef _CASSANDRA_DATABASE_CONTACT_POINTS
#define _CASSANDRA_DATABASE_CONTACT_POINTS "192.168.188.130"
#endif

#ifndef _REDIS_CONTACT_POINTS
#define _REDIS_CONTACT_POINTS "192.168.188.130"
#endif

#ifndef _REDIS_PORT
#define _REDIS_PORT 6379
#endif

#define _PREP_TO_STRING(A) #A
#define PREP_TO_STRING(A) _PREP_TO_STRING(A)

#define _BASH_CHECKMARK "\u2713"
#define _BASH_CROSS "\u2717"
#define _BASH_SUCCESS_MARK "\033[32m[\u2713]:\033[0m "
#define _BASH_FAIL_MARK "\033[31m[\u2717]:\033[0m "
#define _BASH_UNKNOWN_MARK "\033[35m[?]:\033[0m "

#ifdef _SMTP_DEBUG
#define EXCEPT_DEBUG(A) std::string(__FILE__) + std::string(": ") + std::to_string(__LINE__) + std::string("->") + A
#else
#define EXCEPT_DEBUG(A) A
#endif