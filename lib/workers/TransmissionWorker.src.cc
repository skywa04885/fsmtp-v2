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

std::mutex _transmissionMutex;
std::deque<FSMTP::Workers::TransmissionWorkerTask> _transmissionQueue;
extern Json::Value _config;

namespace FSMTP::Workers
{
	/**
	 * Default constructor for the transmission worker
	 *
	 * @Return {void}
	 */
	TransmissionWorker::TransmissionWorker():
		Worker("TransmissionWorker", 120)
	{
	}

	/**
	 * The startup action of the worker
	 *
	 * @Param {void}
	 * @Return {void *} u
	 */
	void TransmissionWorker::startupTask(void)
	{
		const char *hosts = _config["database"]["cassandra_hosts"].asCString();
		const char *username = _config["database"]["cassandra_username"].asCString();
		const char *password = _config["database"]["cassandra_password"].asCString();

		this->w_Logger << "Verbinding maken met: " << hosts << ENDL;
		this->d_Connection = std::make_unique<CassandraConnection>(hosts, username, password);
		this->w_Logger << "Verbinding met database is in stand gebracht !" << ENDL;
	}

	/**
	 * The action that gets performed at interval
	 *
	 * @Param {void *} u
	 */	
	void TransmissionWorker::action(void *u)
	{
		if (_transmissionQueue.size() >= 1)
		{
			TransmissionWorkerTask& task = _transmissionQueue.front();

			try {
				#ifdef _SMTP_DEBUG
				SMTPClient client(false);
				#else
				SMTPClient client(true);
				#endif

				client.prepare(task.t_To, task.t_From, task.t_Content);
				client.beSocial();
			} catch (const std::runtime_error &e)
			{
				this->w_Logger << FATAL << "Could not send email: " << e.what() << ENDL << CLASSIC;
			}

			_transmissionQueue.pop_front();
		}
	}
}