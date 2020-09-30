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

#include "../../default.h"
#include "../../models/Email.src.h"
#include "../../models/Account.src.h"

#define _SMTP_SERV_SESSION_AUTH_FLAG 1
#define _SMTP_SERV_SESSION_SSL_FLAG 2
#define _SMTP_SERV_SESSION_FROM_LOCAL 4
#define _SMTP_SERV_SESSION_RELAY 8
#define _SMTP_SERV_SESSION_SU 16

#define _SMTP_SERV_PA_HELO 1
#define _SMTP_SERV_PA_START_TLS 2
#define _SMTP_SERV_PA_MAIL_FROM 4
#define _SMTP_SERV_PA_RCPT_TO 8
#define _SMTP_SERV_PA_DATA_START 16
#define _SMTP_SERV_PA_DATA_END 32
#define _SMTP_SERV_PA_HELO_AFTER_STLS 64
#define _SMTP_SERV_PA_AUTH_PERF 128

using namespace FSMTP::Models;

namespace FSMTP::Server {
	enum SMTPServerStorageTarget {
		StorageTargetIncomming,
		StorageTargetSent,
		StorageTargetSpam
	};
	
	struct SMTPServerStorageTask {
		AccountShortcut account;
		SMTPServerStorageTarget target;
	};

	struct SMTPServerRelayTask {
		EmailAddress target;
	};

	class SMTPServerSession {
	public:
		SMTPServerSession();

		void setFlag(int64_t mask);
		bool getFlag(int64_t mask);

		void clearAction(int64_t mask);
		void setAction(int64_t mask);
		bool getAction(int64_t mask);

		SMTPServerSession &addStorageTask(const SMTPServerStorageTask &task);
		SMTPServerSession &addRelayTask(const SMTPServerRelayTask &task);
		SMTPServerSession &addTransportTo(const EmailAddress &address);

		const vector<SMTPServerStorageTask> &getStorageTasks();
		const vector<SMTPServerRelayTask> &getRelayTasks();
		const vector<EmailAddress> &getTransportTo();

		SMTPServerSession &setTransformFrom(const EmailAddress &address);

		const EmailAddress& getTransportFrom();

		string &raw();

		SMTPServerSession &setMessageID(const string &id);
		SMTPServerSession &setSubject(const string &subject);
		SMTPServerSession &setSnippet(const string &snippet);
		SMTPServerSession &generateSnippet(const string &raw);

		const string &getMessageID();
		const string &getSubject();
		const string &getSnippet();

		AccountShortcut s_SendingAccount;
		vector<AccountShortcut> s_StorageTasks;
		vector<EmailAddress> s_RelayTasks;
		string s_RawBody;
		bool s_PossSpam;
		FullEmail m_Message;

		~SMTPServerSession();
	private:
		string m_MessageID, m_Subject, m_Snippet;
		vector<EmailAddress> m_TransportTo;
		EmailAddress m_TransportFrom;
		uint64_t m_Date, m_Size;

		vector<SMTPServerStorageTask> m_StorageTasks;
		vector<SMTPServerRelayTask> m_RelayTasks;
		int64_t m_PerformedActions;
		int32_t m_Flags;
		string m_Raw;
	};
}