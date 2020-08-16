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

#ifndef _GENERAL_GLOBAL_H
#define _GENERAL_GLOBAL_H

#include "../default.h"
#include "./connections.src.h"
#include "../networking/sockets/SSLContext.src.h"

using namespace FSMTP::Connections;
using namespace FSMTP::Sockets;

namespace FSMTP
{

  class Global
  {
  public:
    static void configure();
    static void readConfig(const char *config, const char *fallbackConfig);
    static Json::Value &getConfig() noexcept;
    static unique_ptr<CassandraConnection> getCassandra();
    static unique_ptr<RedisConnection> getRedis();
		static unique_ptr<SSLContext> getSSLContext(const SSL_METHOD *method);
  };
}

#endif
