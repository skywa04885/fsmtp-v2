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

#define _EMAIL_FLAG_SEEN 1
#define _EMAIL_FLAG_ANSWERED 2
#define _EMAIL_FLAG_FLAGGED 4
#define _EMAIL_FLAG_DELETED 8
#define _EMAIL_FLAG_DRAFT 16
#define _EMAIL_FLAG_RECENT 32

#include "../default.h"

#include "Email.src.h"
#include "../general/connections.src.h"
#include "../general/macros.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
  class EmailShortcut
  {
  public:
    explicit EmailShortcut(void);

    void save(CassandraConnection *cassandra);

    static vector<EmailShortcut> gatherAll(
      CassandraConnection *cassandra, const int32_t skip,
      int32_t limit, const string &domain,
      const string &mailbox, const CassUuid &uuid
    );

    static vector<tuple<CassUuid, int64_t, int64_t>> gatherAllReferencesWithSize(
      CassandraConnection *cassandra, const int32_t skip,
      int32_t limit, const string &domain,
      const string &mailbox, const CassUuid &uuid,
      const bool deleted
    );

    static pair<int64_t, size_t> getStat(
      CassandraConnection *cassandra, const int32_t skip,
      int32_t limit, const string &domain,
      const string &mailbox, const CassUuid &uuid
    );

    static void deleteOne(
      CassandraConnection *cassandra, const string &domain,
      const CassUuid &ownersUuid, const CassUuid &emailUuid,
      const string &mailbox
    );

    string e_Domain;
    string e_Subject;
    string e_Preview;
    string e_From;
    CassUuid e_OwnersUUID;
    CassUuid e_EmailUUID;
    int32_t e_UID;
    int32_t e_Flags;
    int64_t e_Bucket;
    string e_Mailbox;
    int64_t e_SizeOctets;
  };
}
