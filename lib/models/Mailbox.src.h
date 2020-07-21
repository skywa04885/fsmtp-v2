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

#define _MAILBOX_FLAG_HAS_SUBDIRS 1
#define _MAILBOX_FLAG_UNMARKED 2
#define _MAILBOX_FLAG_ARCHIVE 4
#define _MAILBOX_FLAG_TRASH 8
#define _MAILBOX_FLAG_SENT 16
#define _MAILBOX_FLAG_DRAFT 32
#define _MAILBOX_FLAG_MARKED 64
#define _MAILBOX_FLAG_JUNK 128

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <cassandra.h>

#include "../general/connections.src.h"
#include "../general/exceptions.src.h"
#include "../general/macros.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
	class Mailbox
	{
	public:
		/**
		 * Default empty constructor for MailBox
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit Mailbox(void) noexcept;

		/**
		 * The constructor which litterally makes one
		 *
		 * @Param {const int64_t} e_Bucket
		 * @Param {const std::string &} e_Domain
		 * @Param {const CassUuid &} e_UUID
		 * @Param {const std::String &} e_MailboxPath;
		 * @Param {const bool} e_MailboxStand
		 * @Param {const int32_t} e_MessageCount
		 * @Param {const int32_t} e_Flags
		 * @Param {const bool} e_Subscribed
		 * @Return {void}
		 */
		Mailbox(
			const int64_t e_Bucket,
			const std::string &e_Domain,
			const CassUuid &e_UUID,
			const std::string &e_MailboxPath,
			const bool e_MailboxStand,
			const int32_t e_MessageCount,
			const int32_t e_Flags,
			const bool e_Subscribed
		) noexcept;

		/**
		 * Saves an mailbox to the database
		 *
		 * @Param {CassandraConnection *} cassandra
		 * @Return {void}
		 */
		void save(CassandraConnection *cassandra);

		/**
		 * Gathers all mailboxes from an user
		 *
		 * @Param {CassandraConnection *} cassandra
		 * @Param {const int64_t} bucket
		 * @Param {const std::string &} domain
		 * @Param {CassUuid &} uuid
		 * @Param {const bool} subscribedOnly
		 * @Return {std::vector<Mailbox>}
		 */
		static std::vector<Mailbox> gatherAll(
			CassandraConnection *cassandra,
			const int64_t bucket,
			const std::string &domain,
			const CassUuid &uuid,
			const bool subscribedOnly
		);

		/**
		 * Gets an mailbox
		 *
		 * @Param {CassandraConnection *} cassandra
		 * @Param {const std::string &} domain
		 * @Param {const CassUuid &} uuid
		 * @Param {const std::string &} mailboxPath
		 * @Return {MailboxStatus}
		 */
		static Mailbox get(
			CassandraConnection *cassandra,
			const int64_t bucket,
			const std::string &domain,
			const CassUuid &uuid,
			const std::string &mailboxPath
		);

		int64_t e_Bucket;
		int32_t e_MessageCount;
		std::string e_Domain;
		CassUuid e_UUID;
		std::string e_MailboxPath;
		bool e_MailboxStand;
		bool e_Subscribed;
		int32_t e_Flags;
	};
}