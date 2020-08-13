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
#include "../general/Logger.src.h"

namespace FSMTP::Workers
{
	class Worker
	{
	public:
		Worker(const string &workerName, const size_t w_Interval);
		virtual ~Worker();
		bool start(void *u);
		void stop(void);
		virtual void run(void *u);
		virtual void action(void *u);
		virtual void startupTask(void);

		Logger w_Logger;
	private:
		atomic<bool> w_ShouldRun;
		atomic<bool> w_IsRunning;
		size_t w_Interval;
	};
}