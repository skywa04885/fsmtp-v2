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
	/**
	 * Default constructor for the worker
	 *
	 * @Param {const std::string &} workerName
	 * @Param {const std::size_t &} w_Interval
	 * @Return {void}
	 */
	Worker::Worker(const std::string &workerName, const std::size_t w_Interval):
		w_ShouldRun(false), w_IsRunning(false),
		w_Logger(workerName, LoggerLevel::INFO),
		w_Interval(w_Interval)
	{
		this->w_Logger << "Worker aangemaakt" << ENDL;
	}

	/**
	 * Starts the worker, u is used for some user
	 * - data, which is sometimes required
	 *
	 * @Param {void *} u
	 * @Return {void}
	 */
	bool Worker::start(void *u)
	{
		// Starts the worker by setting the
		// - should run to true, and creating
		// - the worker thread, and detaching
		// - it, since we're otherwise going to wait
		this->w_Logger << "Worker wordt opgestart ... " << ENDL;
		this->w_Logger << "Worker opstart opdracht wordt uitgevoerd ..." << ENDL;
		try {
			this->startupTask();
		}
		catch (const std::runtime_error &e)
		{
			this->w_Logger << ERROR << "Worker kon niet worden gestart: " << e.what() << ENDL << CLASSIC;
			return false;
		}
		this->w_Logger << "Worker opstart opdracht is uitgevoerd !" << ENDL;

		this->w_ShouldRun = true;
		std::thread t(&Worker::run, this, u);
		t.detach();

		this->w_Logger << "Worker opgestart !" << ENDL;
		return true;
	}

	/**
	 * Stops the worker
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void Worker::stop(void)
	{
		// Shuts the worker down by
		// - setting the should run to false
		// - and waiting for the isRunning
		// - to be set to false, next to
		// - that we keep track of the
		// - shutdown time
		this->w_Logger << "Worker wordt afgesloten ..." << ENDL;
		int64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();

		this->w_ShouldRun = false;
		while (this->w_IsRunning)
			std::this_thread::sleep_for(std::chrono::milliseconds(40));

		int64_t endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
		this->w_Logger << "Worker afgesloten in " << endTime - startTime << " milliseconden !" << ENDL;
	}

	/**
	 * Creates and runs the thread
	 *
	 * @Param {void *} u
	 * @Return {void}
	 */
	void Worker::run(void *u)
	{
		// Sets running to true,
		// - and keeps running as long
		// - as we do not stop, and if we stop
		// - we set running to false
		this->w_IsRunning = true;
		while(this->w_ShouldRun)
		{
			this->action(u);
			std::this_thread::sleep_for(std::chrono::milliseconds(this->w_Interval));
		}
		this->w_IsRunning = false;
	}

	/**
	 * The action which needs to perform at the
	 * - specified interval
	 *
	 * @Param {void *} u
	 * @Return {void}
	 */
	void Worker::action(void *u)
	{
		std::cout << "Worker::action() is not implemented !" << std::endl;
	}

	/**
	 * An task which needs to be performed at
	 * - the start of the worker
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void Worker::startupTask(void)
	{
		std::cout << "Worker::startupTask() is not implemented !" << std::endl;
	}
}