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

#include "Models.src.h"
#include "Mailbox.src.h"
#include "EmailShortcut.src.h"

namespace FSMTP::Models
{
	class MailboxStatus
	{
	public:
		/**
		 * Empty constructor for the mailbox status
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit MailboxStatus(void);

		/**
		 * Gets the mailbox status
		 *
		 * @Param {RedisConnection *} redis
	 	 * @Param {CassandraConnection *} cassandra
		 * @Param {const int64_t} s_Bucket
		 * @Param {const std::string &} s_Domain
		 * @Param {const CassUuid &} uuid
		 * @Param {const std::string &} mailboxPath
		 */
		static MailboxStatus get(
			RedisConnection *redis,
			CassandraConnection *cassandra,
			const int64_t s_Bucket,
			const std::string &s_Domain,
			const CassUuid &uuid,
			const std::string &mailboxPath
		);

		/**
		 * Adds an new message to an mailbox
		 *
		 * @Param {RedisConnection *} redis
	 	 * @Param {CassandraConnection *} cassandra
		 * @Param {const int64_t} s_Bucket
		 * @Param {const std::string &} s_Domain
		 * @Param {const CassUuid &} uuid
		 * @Param {const std::string &} mailboxPath
	 	 * @Return {int32_t} uid
		 */
		static int32_t addOneMessage(
			RedisConnection *redis,
			CassandraConnection *cassandra,
			const int64_t s_Bucket,
			const std::string &s_Domain,
			const CassUuid &uuid,
			const std::string &mailboxPath
		);

		/**
		 * Saves the mailbox status
		 *
		 * @Param {RedisConnection *} redis
		 * @Return {void}
		 */
		void save(RedisConnection *redis, const std::string &mailboxPath);

		/**
		 * Restores an mailbox status from cassandra (EXPENSIVE)
		 *
		 * @Param {CassandraConnection *} cassandra
		 * @Param {const std::string &} domain
		 * @Param {const CassUuid &} uuid
		 * @Param {const std::string &} mailboxPath
		 * @Return {MailboxStatus}
		 */
		static MailboxStatus restoreFromCassandra(
			CassandraConnection *cassandra,
			const int64_t bucket,
			const std::string &domain,
			const CassUuid &uuid,
			const std::string &mailboxPath
		);

		int64_t s_Bucket;
		std::string s_Domain;
		CassUuid s_UUID;
		int32_t s_Unseen;
		int32_t s_NextUID;
		int32_t s_Recent;
		int32_t s_Total;
		int32_t s_Flags;
		int32_t s_PerfmaFlags;
	};
}