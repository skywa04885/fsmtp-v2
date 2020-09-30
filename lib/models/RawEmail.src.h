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
#include "../general/connections.src.h"
#include "../general/exceptions.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models {
  class RawEmail {
  public:
    RawEmail();
    RawEmail(int64_t bucket, const string &domain, const CassUuid &owner,
      const CassUuid &email, const string &content);

    void save(CassandraConnection *cassandra);

    static RawEmail get(
      CassandraConnection *cassandra, const string &domain,
      const CassUuid &ownersUuid, const CassUuid &emailUuid,
      const int64_t bucket
    );

    static void deleteOne(
      CassandraConnection *cassandra, const string &domain,
      const CassUuid &ownersUuid, const CassUuid &emailUuid,
      const int64_t bucket
    );

    int64_t e_Bucket;
    string e_Domain;
    CassUuid e_OwnersUUID;
    CassUuid e_EmailUUID;
    string e_Content;
  };
}
