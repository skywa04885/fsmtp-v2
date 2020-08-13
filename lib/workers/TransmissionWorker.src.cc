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
deque<FSMTP::Workers::TransmissionWorkerTask> _transmissionQueue;

namespace FSMTP::Workers
{
	TransmissionWorker::TransmissionWorker():
		Worker("TransmissionWorker", 120)
	{}

	void TransmissionWorker::startupTask(void) {
		this->d_Connection = Global::getCassandra();
	}

	void TransmissionWorker::action(void *u) {
		if (_transmissionQueue.size() >= 1) {
			TransmissionWorkerTask& task = _transmissionQueue.front();

			try {
				#ifdef _SMTP_DEBUG
				SMTPClient client(false);
				#else
				SMTPClient client(true);
				#endif

				client.prepare(task.t_To, task.t_From, task.t_Content);
				client.beSocial();
			} catch (const runtime_error &e) {
				this->w_Logger << FATAL << "Could not send email: " << e.what() << ENDL << CLASSIC;
			}

			_transmissionQueue.pop_front();
		}
	}
}