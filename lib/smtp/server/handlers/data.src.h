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

#ifndef _LIB_SMTP_SERVER_MESSAGE_HANDLER_H
#define _LIB_SMTP_SERVER_MESSAGE_HANDLER_H

#include "../../../default.h"
#include "../SMTPServerSession.src.h"
#include "../../Response.src.h"
#include "../../../spf/SPFValidator.src.h"
#include "../../../dkim/DKIMValidator.src.h"
#include "../../../dmarc/DMARCRecord.src.h"
#include "../../../networking/sockets/ClientSocket.src.h"
#include "../../../mime/mimev2.src.h"
#include "../../../builders/mimev2.src.h"
#include "../SMTPHeaders.src.h"
#include "../../../workers/DatabaseWorker.src.h"
#include "../../../workers/TransmissionWorker.src.h"
#include "../SMTPServerExceptions.src.h"

using namespace FSMTP::Server;
using namespace FSMTP::Sockets;

namespace FSMTP::SMTP::Server::Handlers {
    bool dataHandler(
		shared_ptr<ClientSocket> client,
        shared_ptr<SMTPServerSession> session,
		Logger &clogger
    );
}

#endif
