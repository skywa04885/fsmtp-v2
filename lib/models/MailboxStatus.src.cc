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

#include "MailboxStatus.src.h"

namespace FSMTP::Models
{
	/**
	 * Empty constructor for the mailbox status
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	MailboxStatus::MailboxStatus(void)
	{}

	/**
	 * Restores an mailbox status from cassandra (EXPENSIVE)
	 *
	 * @Param {CassandraConnection *} cassandra
	 * @Param {const std::string &} domain
	 * @Param {const CassUuid &} uuid
	 * @Param {const std::string &} mailboxPath
	 * @Return {MailboxStatus}
	 */
	MailboxStatus MailboxStatus::restoreFromCassandra(
		CassandraConnection *cassandra,
		const int64_t bucket,
		const std::string &domain,
		const CassUuid &uuid,
		const std::string &mailboxPath
	)
	{
		MailboxStatus res;
		res.s_Bucket = bucket;
		res.s_Domain = domain;
		res.s_UUID = cass_uuid_timestamp(uuid);

		// Gets the mailbox itself
		Mailbox mailbox = Mailbox::get(cassandra, bucket, domain, uuid, mailboxPath);
		res.s_Total = mailbox.e_MessageCount;

		// Starts looping over the messages, and checking
		// - the flags
	}
}