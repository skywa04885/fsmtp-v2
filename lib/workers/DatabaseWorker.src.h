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
#include "../general/Global.src.h"

#include "../models/Email.src.h"
#include "../models/RawEmail.src.h"
#include "../models/EmailShortcut.src.h"
#include "./Worker.src.h"
#include "../general/connections.src.h"
#include "../models/MailboxStatus.src.h"

using namespace FSMTP::Models;
using namespace FSMTP::Connections;

namespace FSMTP::Workers
{
	class DatabaseWorker : public Worker
	{
	public:
		DatabaseWorker(void);
		virtual void startupTask(void);
		virtual void action(void *u);
	private:
		unique_ptr<CassandraConnection> d_Cassandra;
		unique_ptr<RedisConnection> d_Redis;
	};
}
