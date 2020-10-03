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

#ifndef _LIB_DEFAULT_H
#define _LIB_DEFAULT_H

// ==== STL/C++ Libraries ====
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <regex>
#include <cstdint>
#include <functional>
#include <filesystem>
#include <fstream>
#include <tuple>
#include <algorithm>
#include <memory>
#include <thread>
#include <atomic>
#include <list>
#include <chrono>
#include <random>
#include <unordered_map>
#include <future>

// ==== Other C++ Library's ====
#include <json/json.h>
#include <cassandra.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <boost/multiprecision/cpp_int.hpp>

// ==== C Libraries ====
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <string.h>
#include <assert.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <unistd.h>

// ==== Using namespaces ====
using namespace std;
using namespace chrono;
using namespace nlohmann;

// ==== Global functions/classes ====

string __ssl_get_error();
string __except_debug(const char *file, const size_t line, const string &mess);

template<typename T>
class Defer
{
public:
	T cb;
	Defer(T cb): cb(cb) {}
	~Defer() { cb(); }
};

template<typename T>
Defer<T> __defer_func(T cb)
{ return Defer<T>(cb); }


typedef struct __attribute__ (( packed )) {
	uint64_t seg0;
	uint64_t seg1;
} uint128_t;

typedef vector<string>::iterator strvec_it;

// ==== Global definitions ====

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define DEFER_M(code) auto DEFER_3(_defer_) = __defer_func([&]()code)
#define DEFER(code) auto DEFER_3(_defer_) = __defer_func([&](){code;})

#define SSL_STRERROR __ssl_get_error()

#define CUSTOM_EXCEPTION(name) \
class name : public exception { \
public: \
	name(const string &err): err(err) {} \
	const char *what() const throw() { return this->err.c_str(); } \
private: string err; \
};

#define BINARY_COMPARE(A, B) (A & B) == B

#define _PREP_TO_STRING(A) #A
#define PREP_TO_STRING(A) _PREP_TO_STRING(A)

#define _BASH_CHECKMARK "\u2713"
#define _BASH_CROSS "\u2717"
#define _BASH_SUCCESS_MARK "\033[32m[\u2713]:\033[0m "
#define _BASH_FAIL_MARK "\033[31m[\u2717]:\033[0m "
#define _BASH_UNKNOWN_MARK "\033[35m[?]:\033[0m "

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

#ifdef _SMTP_DEBUG
#define EXCEPT_DEBUG(A) __except_debug(__FILE__, __LINE__, A)
#else
#define EXCEPT_DEBUG(A) A
#endif

#define FATAL_ERROR(A) std::cerr << "\033[32m" << __LINE__ << '@' << __FILE__ << "\033[31m" << ": " << A << "\033[0m" << std::endl

#define _BV(A) (1 << A)

#endif
