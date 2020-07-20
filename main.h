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

#include "lib/general/AES256.src.h"
#include "lib/args/args.src.h"
#include "lib/smtp/server/SMTPServer.src.h"
#include "lib/smtp/client/SMTPClient.src.h"
#include "lib/workers/DatabaseWorker.src.h"
#include "lib/workers/TransmissionWorker.src.h"

#include "lib/pop3/P3Server.src.h"
#include "lib/imap/IMAPServer.src.h"

using namespace FSMTP;
using Networking::SMTPSocket;
using Server::SMTPServer;
using namespace FSMTP::Workers;
using namespace FSMTP::Mailer::Composer;
using namespace FSMTP::Mailer::Client;

int main(const int argc, const char **argv);
