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

#ifndef _LIB_MODELS_UID_HOLDER
#define _LIB_MODELS_UID_HOLDER

#include "../default.h"
#include "../general/connections.src.h"
#include "../general/exceptions.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models {
  class UIDHolder {
  public:
		static void getPrefix(
			const int64_t bucket, const string &domain,
  		const CassUuid &uuid, char *buffer
		);

		static int32_t getAndIncrement(
			CassandraConnection *cass, RedisConnection *redis,
  		const int64_t bucket, const string &domain,
  		const CassUuid &uuid
		);

		static int32_t getRedis(
			RedisConnection *redis, const int64_t bucket,
			const string &domain, const CassUuid &uuid
		);

		static void saveRedis(
			RedisConnection *redis, const int64_t bucket,
			const string &domain, const CassUuid &uuid,
			const int32_t num
		);

  	static int32_t restoreFromCassandra(
  		CassandraConnection *cass, RedisConnection *redis,
  		const int64_t bucket, const string &domain,
  		const CassUuid &uuid
  	);
  	static int32_t increment();
  };
}

#endif