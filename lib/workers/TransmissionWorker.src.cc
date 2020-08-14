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


#include "TransmissionWorker.src.h"

static mutex queueMutex;
static deque<shared_ptr<SMTPServerSession>> transmissionQueue;

namespace FSMTP::Workers
{
	TransmissionWorker::TransmissionWorker():
		Worker("TRANSMITTER", 120)
	{}

	void TransmissionWorker::startupTask(void) {
		auto &logger = this->w_Logger;

		this->d_Connection = Global::getCassandra();
		logger << _BASH_SUCCESS_MARK << "Connected to cassandra" << ENDL;
	}

	void TransmissionWorker::push(shared_ptr<SMTPServerSession> session) {
		queueMutex.lock();
		transmissionQueue.push_back(session);
		queueMutex.unlock();
	}

	static inja::Environment env;
	static inja::Template errorTemplate = env.parse_template("../templates/mailer/error.html");
	void TransmissionWorker::sendErrorsToSender(const SMTPClient &client) {
		json data = json::object();
		data["subject"] = "SMTP Delivery failure";
		data["errors"] = client.s_ErrorLog;

		string result = env.render(errorTemplate, data);

		MailComposerConfig composeConfig;
		composeConfig.m_BodySections.push_back(EmailBodySection {
			result, EmailContentType::ECT_TEXT_HTML,
			{}, 0, EmailTransferEncoding::ETE_QUOTED_PRINTABLE
		});
		composeConfig.m_From.push_back(EmailAddress(
			"FSMTP Delivery", 
			"test@gmail.com"
		));
		composeConfig.m_To.push_back(client.s_MailFrom);
		composeConfig.m_Subject = "SMTP Delivery failure";
		
		SMTPClient mailer(true);
		mailer.prepare(composeConfig).beSocial();
	}

	void TransmissionWorker::action(void *u) {
		while (transmissionQueue.size() > 0) {

			// Gets the first task in the list, the task will tell the
			//  transmitter where to transmit the message to

			queueMutex.lock();
			shared_ptr<SMTPServerSession> task = transmissionQueue.front();
			queueMutex.unlock();

			// Attempts to send the message to the client/clients, if this fails
			//  we will send an error notice to the transmitters mailbox

			auto &to = task->s_RelayTasks;
			auto &from = task->s_TransportMessage.e_TransportFrom;
			auto &content = task->s_RawBody;

			try {
				#ifdef _SMTP_DEBUG
				SMTPClient client(false);
				#else
				SMTPClient client(true);
				#endif

				client.prepare(to, { from }, content).beSocial();

				if (client.s_ErrorCount > 0) {
					sendErrorsToSender(client);
				}
			} catch (const runtime_error &e) {
				this->w_Logger << FATAL << "Could not send email: " << e.what() << ENDL << CLASSIC;
			}

			transmissionQueue.pop_front();
		}
	}
}