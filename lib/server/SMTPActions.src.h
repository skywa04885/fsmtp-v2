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

#include <cstdint>
#include <stdexcept>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../smtp/Command.src.h"
#include "../smtp/Response.src.h"
#include "../networking/SMTPSocket.src.h"

using namespace FSMTP::SMTP;
using namespace FSMTP::Networking;

namespace FSMTP::Server::Actions
{
	void actionHelloInitial(
		const ClientCommand &command,
		int32_t &fd, 
		struct sockaddr_in *sAddr,
		const bool &ssl, 
		const bool &esmtp
	);

	void actionMailFrom(
		const ClientCommand &command,
		int32_t &fd, 
		struct sockaddr_in *sAddr,
		const bool &ssl, 
		const bool &esmtp
	);
}
