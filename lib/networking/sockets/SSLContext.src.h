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

#ifndef _LIB_NETWORKING_SOCKETS_SSL_CONTEXT_H
#define _LIB_NETWORKING_SOCKETS_SSL_CONTEXT_H

#include "../../default.h"
#include "../../general/macros.src.h"

namespace FSMTP::Sockets {
  class SSLContext {
  public:
    SSLContext() noexcept;
		~SSLContext() noexcept;

		SSLContext &justCreate();
    SSLContext &read(const char *privateKey, const char *cert);
		SSLContext &password(const char * );
		SSLContext &method(const SSL_METHOD *method);
    
		static int32_t passwordCallback(
			char *buffer, int32_t size, 
			int32_t rwflag, void *u
		);

    SSL_CTX *p_SSLCtx;
		const SSL_METHOD *p_SSLMethod;
		string p_Password;
  };
}

#endif