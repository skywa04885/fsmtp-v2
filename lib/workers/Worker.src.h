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

#include <vector>
#include <thread>
#include <atomic>
#include <string>
#include <chrono>

#include "../general/logger.src.h"

namespace FSMTP::Workers
{
	class Worker
	{
	public:
		/**
		 * Default constructor for the worker
		 *
		 * @Param {const std::string &} workerName
		 * @Param {const std::size_t &} w_Interval
		 * @Return {void}
		 */
		Worker(const std::string &workerName, const std::size_t w_Interval);

		/**
		 * Starts the worker, u is used for some user
		 * - data, which is sometimes required
		 *
		 * @Param {void *} u
		 * @Return {void}
		 */
		bool start(void *u);

		/**
		 * Stops the worker
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		void stop(void);

		/**
		 * Creates and runs the thread
		 *
		 * @Param {void *} u
		 * @Return {void}
		 */
		virtual void run(void *u);

		/**
		 * The action which needs to perform at the
		 * - specified interval
		 *
		 * @Param {void *} u
		 * @Return {void}
		 */
		virtual void action(void *u);

		/**
		 * An task which needs to be performed at
		 * - the start of the worker
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		virtual void startupTask(void);

		Logger w_Logger;
	private:
		std::atomic<bool> w_ShouldRun;
		std::atomic<bool> w_IsRunning;
		std::size_t w_Interval;
	};
}