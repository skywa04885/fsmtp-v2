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

#include "../models/Email.src.h"
#include "../models/Account.src.h"
#include "../models/EmailShortcut.src.h"
#include "../smtp/client/SMTPClient.src.h"
#include "./Worker.src.h"
#include "../general/connections.src.h"
#include "../smtp/server/SMTPServerSession.src.h"

using namespace FSMTP::Models;
using namespace FSMTP::Connections;
using namespace FSMTP::Mailer::Client;
using namespace FSMTP::Server;

namespace FSMTP::Workers
{
	class TransmissionWorker : public Worker {
	public:
		TransmissionWorker();
		virtual void startupTask(void);
		virtual void action(void *u);

		static void sendErrorsToSender(SMTPClient &client);
		static void push(shared_ptr<SMTPServerSession> session);
	private:
		unique_ptr<CassandraConnection> m_Cassandra;
	};
}
