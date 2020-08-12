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
#include <cstdint>
#include <functional>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <memory>
#include <thread>
#include <chrono>

// ==== Other C++ Library's ====
#include <json/json.h>
#include <cassandra.h>

// ==== C Libraries ====
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <string.h>

// ==== Using namespaces ====
using namespace std;
using namespace chrono;

// ==== Global functions/classes ====
string __ssl_get_error();

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

// ==== Global definitions ====

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define DEFER_M(code) auto DEFER_3(_defer_) = __defer_func([&]()code)
#define DEFER(code) auto DEFER_3(_defer_) = __defer_func([&](){code;})

#define SSL_STRERROR __ssl_get_error()

#endif