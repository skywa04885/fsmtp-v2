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

		this->m_Cassandra = Global::getCassandra();
		logger << _BASH_SUCCESS_MARK << "Connected to cassandra" << ENDL;
	}

	void TransmissionWorker::push(shared_ptr<SMTPServerSession> session) {
		queueMutex.lock();
		transmissionQueue.push_back(session);
		queueMutex.unlock();
	}

	static inja::Environment env;
	static inja::Template errorTemplate = env.parse_template("../templates/mailer/error.html");
	void TransmissionWorker::sendErrorsToSender(SMTPClient &client) {
		// Builds the input data for the inja templating engine,
		//  this will contain the errors the server captured
		json data = json::object();
		data["subject"] = "SMTP Delivery failure";
		data["errors"] = client.s_ErrorLog;

		// Renders the template, and stores the result into a
		//  string
		string result = env.render(errorTemplate, data);

		// Creates the message composer config, this will be used to compose
		//  the message, which will be transmitted to the sender
		MailComposerConfig composeConfig;
		composeConfig.m_BodySections.push_back(EmailBodySection {
			result, EmailContentType::ECT_TEXT_HTML,
			{}, 0, EmailTransferEncoding::ETE_QUOTED_PRINTABLE
		});
		composeConfig.m_From.push_back(EmailAddress("Delivery Subsystem",  "delivery@fannst.nl"));
		composeConfig.m_To.push_back(client.s_MailFrom);
		composeConfig.m_Subject = "SMTP Delivery failure";

		// Resets the SMTPClient, prepares it and tells it to be social
		client.reset().prepare(composeConfig).beSocial();
	}

	void TransmissionWorker::action(void *u) {
		auto *cassandra = this->m_Cassandra.get();
		auto &logger = this->w_Logger;

		for (;;) {
			// ============================
			// Gets the front of the queue
			// ============================

			queueMutex.lock(); // Locks the transmission mutex

			// Checks if there is anything to transmit, else just
			//  exit the worker action and wait
			if (transmissionQueue.size() <= 0) {
				queueMutex.unlock();
				break;
			}

			// Gets the last task from the queue, and pops it of it
			//  so we will not have to process it again
			shared_ptr<SMTPServerSession> session = transmissionQueue.back();
			transmissionQueue.pop_back();

			queueMutex.unlock(); // Unlocks the transmission queue mutex

			// ============================
			// Transmits the email
			// ============================

			try {
				// Creates an SMTP client, if we're in debug
				//  we will enable verbose
				#ifdef _SMTP_DEBUG
				SMTPClient client(true);
				#else
				SMTPClient client(false);
				#endif

				// Builds the vector of addresses we will transmit the
				//  message to, this is required for the prepare method
				vector<EmailAddress> to = {};
				auto &tasks = session->getRelayTasks();
				for_each(tasks.begin(), tasks.end(), [&](const SMTPServerRelayTask &task) {
					to.push_back(task.target);
				});

				// Prints the debug message to the console, which will
				//  tell that we're transmitting one message
				DEBUG_ONLY(logger << DEBUG << "Transmitting message to " << to.size() << " targets .." 
					<< ENDL << CLASSIC);

				// Prepares the client for the message transmission, after which we tell
				//  the client to be social, and talk to the servers
				client.prepare(to, {
					session->getTransportFrom()
				}, session->raw()).beSocial();

				// Checks if there were any errors, if so transmit an error message to the
				//  sender, since he/she will otherwise not know something went wrong.
				if (client.s_ErrorCount > 0) {
					DEBUG_ONLY(logger << ERROR << "Transmission failed, sending error message to sender .." << ENDL << CLASSIC);
					this->sendErrorsToSender(client);
				} else {
					// Prints the debug message that we successfully transmitted to the clients
					DEBUG_ONLY(logger << DEBUG << "Transmission to " << to.size() << " targets was successfull !" << ENDL << CLASSIC);
				}
			} catch (const runtime_error &e) {
				logger << ERROR << "Failed send message, runtime error: " << e.what() << ENDL << CLASSIC;
			} catch (...) {
				logger << ERROR << "Failed send message, unknown error" << ENDL << CLASSIC;
			}
		}
	}
}