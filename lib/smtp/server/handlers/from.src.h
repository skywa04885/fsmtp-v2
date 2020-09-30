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

#ifndef _LIB_SMTP_SERVER_HANDLERS_FROM_H
#define _LIB_SMTP_SERVER_HANDLERS_FROM_H

#include "../../../default.h"
#include "../../../networking/sockets/ClientSocket.src.h"
#include "../../../networking/sockets/SSLContext.src.h"
#include "../SMTPServerSession.src.h"
#include "../../Command.src.h"

using namespace FSMTP::Server;
using namespace FSMTP::Sockets;

namespace FSMTP::SMTP::Server::Handlers {
    bool fromHandler(
        shared_ptr<ClientSocket> client,
        shared_ptr<SMTPServerSession> session,
        const ClientCommand &command
    );
}

#endif
