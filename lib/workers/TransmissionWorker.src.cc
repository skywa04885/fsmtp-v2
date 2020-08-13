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

mutex _transmissionMutex;
extern mutex _emailStorageMutex;
extern vector<pair<string, FullEmail>> _emailStorageQueue;
deque<FSMTP::Workers::TransmissionWorkerTask> _transmissionQueue;

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

	void TransmissionWorker::action(void *u) {
		if (_transmissionQueue.size() >= 1) {

			// Gets the first task in the list, the task will tell the
			//  transmitter where to transmit the message to

			_transmissionMutex.lock();
			TransmissionWorkerTask& task = _transmissionQueue.front();
			_transmissionMutex.unlock();

			// Attempts to send the message to the client/clients, if this fails
			//  we will send an error notice to the transmitters mailbox

			try {
				#ifdef _SMTP_DEBUG
				SMTPClient client(false);
				#else
				SMTPClient client(true);
				#endif

				client.prepare(task.t_To, task.t_From, task.t_Content).beSocial();
			} catch (const runtime_error &e) {
				this->w_Logger << FATAL << "Could not send email: " << e.what() << ENDL << CLASSIC;
			}

			_transmissionQueue.pop_front();
		}
	}
}