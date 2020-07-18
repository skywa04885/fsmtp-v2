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

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "../models/Email.src.h"
#include "../models/LocalDomain.src.h"
#include "../general/connections.src.h"
#include "../networking/SMTPSocket.src.h"
#include "../pop3/P3Server.src.h"
#include "../general/Logger.src.h"
#include "../general/macros.src.h"
#include "../general/connections.src.h"
#include "../general/AES256.src.h"
#include "../smtp/server/SMTPServer.src.h"
#include "../smtp/client/SMTPClient.src.h"

using namespace FSMTP::Networking;
using namespace FSMTP::Server;
using namespace FSMTP::Mailer::Client;
using namespace FSMTP::Connections;

namespace FSMTP::ARG_ACTIONS
{
  /**
   * Performs an default set of tests, which will check if the Server
   * - is capable of connecting to the database etcetera
   *
   * @Param {void}
   * @Return {void}
   */
  void testArgAction(void);

  /**
   * Adds an user to the database
   *
   * @Param {void}
   * @Return {void}
   */
  void addUserArgAction(void);

  /**
   * Syncs cassandra with redis
   *
   * @Param {void}
   * @Return {void}
   */
  void syncArgAction(void);

  /**
   * Sends an email custom or default
   *
   * @Param {void}
   * @Return {void}
   */
  void mailTestArgAction(void);
}
