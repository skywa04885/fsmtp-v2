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

#include "Worker.src.h"

namespace FSMTP::Workers
{
	Worker::Worker(const string &workerName, const size_t w_Interval):
		w_ShouldRun(false), w_IsRunning(false),
		w_Logger(workerName, LoggerLevel::INFO),
		w_Interval(w_Interval)
	{
		this->w_Logger << "Worker aangemaakt" << ENDL;
	}

	Worker::~Worker() {
		if (this->w_IsRunning) {
			this->stop();
		}
	}

	bool Worker::start(void *u) {
		auto &logger = this->w_Logger;

		// Starts the worker by setting the
		//  should run to true, and creating
		//  the worker thread, and detaching
		//  it, since we're otherwise going to wait

		logger << "Worker wordt opgestart" << ENDL;
		logger << "Worker opstart opdracht wordt uitgevoerd" << ENDL;
		try {
			this->startupTask();
		} catch (const runtime_error &e) {
			this->w_Logger << ERROR << "Worker kon niet worden gestart: " << e.what() << ENDL << CLASSIC;
			return false;
		}
		logger << "Worker opstart opdracht is uitgevoerd !" << ENDL;

		this->w_ShouldRun = true;
		thread(&Worker::run, this, u).detach();

		logger << "Worker opgestart !" << ENDL;
		return true;
	}

	void Worker::stop(void) {
		auto &logger = this->w_Logger;

		// Shuts the worker down by
		//  setting the should run to false
		//  and waiting for the isRunning
		//  to be set to false, next to
		//  that we keep track of the
		//  shutdown time

		logger << "Worker wordt afgesloten ..." << ENDL;
		int64_t startTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();

		this->w_ShouldRun = false;
		while (this->w_IsRunning) {
			this_thread::sleep_for(milliseconds(40));
		}

		int64_t endTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		logger << "Worker afgesloten in " << endTime - startTime << " milliseconden !" << ENDL;
	}

	void Worker::run(void *u) {
		auto &logger = this->w_Logger;
		auto &isRunning = this->w_IsRunning;
		auto &shouldRun = this->w_ShouldRun;

		// Sets running to true,
		//  and keeps running as long
		//  as we do not stop, and if we stop
		//  we set running to false

		isRunning = true;
		while(shouldRun) {
			try {
				this->action(u);
			} catch (const EmptyQuery &e) {
				logger << ERROR << "Execution failed: " << e.what() << ENDL << CLASSIC;
			} catch (...) {
				logger << ERROR << "Execution failed with unknown error !" << ENDL << CLASSIC;
			}

			this_thread::sleep_for(milliseconds(this->w_Interval));
		}
		isRunning = false;
	}

	void Worker::action(void *u) {
		cout << "Worker::action() is not implemented !" << endl;
	}

	void Worker::startupTask(void) {
		cout << "Worker::startupTask() is not implemented !" << endl;
	}
}
