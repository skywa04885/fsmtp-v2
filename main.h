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
#include <memory>
#include <cstdint>
#include <bitset>
#include <vector>
#include <string>

#include <openssl/ssl.h>

#include "lib/networking/SMTPSocket.src.h"
#include "lib/server/SMTPServer.src.h"
#include "lib/smtp/Response.src.h"
#include "lib/models/email.src.h"
#include "lib/general/connections.src.h"
#include "lib/general/macros.src.h"
#include "lib/general/logger.src.h"
#include "lib/general/encoding.src.h"
#include "lib/general/NCursesDisplay.src.h"
#include "lib/workers/DatabaseWorker.src.h"
#include "lib/workers/TransmissionWorker.src.h"
#include "lib/client/SMTPMessageComposer.src.h"
#include "lib/client/SMTPClient.src.h"
#include "lib/networking/DNS.src.h"

using namespace FSMTP;
using Networking::SMTPSocket;
using Server::SMTPServer;
using namespace FSMTP::Workers;
using namespace FSMTP::Mailer::Composer;
using namespace FSMTP::Mailer::Client;

int main(const int argc, const char **argv);
