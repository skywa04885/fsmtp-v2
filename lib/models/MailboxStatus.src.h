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

#include "../default.h"
#include "Mailbox.src.h"
#include "EmailShortcut.src.h"

namespace FSMTP::Models
{
	class MailboxStatus
	{
	public:
		explicit MailboxStatus();

		static void clearRecent(
			RedisConnection *redis, const int64_t s_Bucket,
			const string &s_Domain, const CassUuid &uuid,
			const string &mailboxPath
		);
		static MailboxStatus get(
			RedisConnection *redis, CassandraConnection *cassandra,
			const int64_t s_Bucket, const string &s_Domain,
			const CassUuid &uuid, const string &mailboxPath
		);

		static int32_t addOneMessage(
			RedisConnection *redis, CassandraConnection *cassandra,
			const int64_t s_Bucket, const string &s_Domain,
			const CassUuid &uuid, const string &mailboxPath
		);

		void save(RedisConnection *redis, const string &mailboxPath);

		static MailboxStatus restoreFromCassandra(
			CassandraConnection *cassandra, const int64_t bucket,
			const string &domain, const CassUuid &uuid,
			const string &mailboxPath
		);

		int64_t s_Bucket;
		string s_Domain;
		CassUuid s_UUID;
		int32_t s_Unseen;
		int32_t s_NextUID;
		int32_t s_Recent;
		int32_t s_Total;
		int32_t s_Flags;
		int32_t s_PerfmaFlags;
	};
}