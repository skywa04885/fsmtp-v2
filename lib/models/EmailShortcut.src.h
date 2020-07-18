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

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstdint>

#include <cassandra.h>

#include "Email.src.h"
#include "../general/connections.src.h"
#include "../general/macros.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
  class EmailShortcut
  {
  public:
    /**
     * Default empty constructor for the EmailShortcut class
     *
     * @Param {void}
     * @Return {void}
     */
    explicit EmailShortcut(void);

    /**
     * Stores an email shortcut in the cassandra database
     *
     * @Param {CassandraConnection *} cassandra
     * @Return {void}
     */
    void save(CassandraConnection *cassandra);

    /**
     * Gathers all messages from an specific user
     *
     * @Param {CassandraConnection *} cassandra
     * @Param {const int32_t} skip
     * @Param {int32_t} limit
     * @Param {const std::string &} domain
     * @Param {const CassUuid &} uuid
     * @Return {std::vector<EmailShortcut>}
     */
    static std::vector<EmailShortcut> gatherAll(
      CassandraConnection *cassandra,
      const int32_t skip,
      int32_t limit,
      const std::string &domain,
      const CassUuid &uuid
    );

    std::string e_Domain;
    std::string e_Subject;
    std::string e_Preview;
    CassUuid e_OwnersUUID;
    CassUuid e_EmailUUID;
    int64_t e_Bucket;
    EmailType e_Type;
    int64_t e_SizeOctets;
  };
}
