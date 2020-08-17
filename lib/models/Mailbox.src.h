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
#define _MAILBOX_SYS_FLAG_READ_ONLY 256

#define _MAILBOX_FLAG_COUNT 9

#include "../default.h"
#include "../general/connections.src.h"
#include "../general/exceptions.src.h"
#include "../general/macros.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
	class Mailbox
	{
	public:
		explicit Mailbox(void) noexcept;

		Mailbox(
			const int64_t e_Bucket, const string &e_Domain,
			const CassUuid &e_UUID, const string &e_MailboxPath,
			const bool e_MailboxStand, const int32_t e_MessageCount,
			const int32_t e_Flags, const bool e_Subscribed
		) noexcept;

		void save(CassandraConnection *cassandra);

		static vector<Mailbox> gatherAll(
			CassandraConnection *cassandra, const int64_t bucket,
			const string &domain, const CassUuid &uuid,
			const bool subscribedOnly
		);
		static Mailbox get(
			CassandraConnection *cassandra, const int64_t bucket,
			const string &domain, const CassUuid &uuid,
			const string &mailboxPath
		);

		int64_t e_Bucket;
		int32_t e_MessageCount;
		string e_Domain;
		CassUuid e_UUID;
		string e_MailboxPath;
		bool e_MailboxStand;
		bool e_Subscribed;
		int32_t e_Flags;
	};
}