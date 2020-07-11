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


#include "DatabaseWorker.src.h"

std::vector<FullEmail> _emailStorageQueue = {};

namespace FSMTP::Workers
{
	DatabaseWorker::DatabaseWorker(const std::string &d_ContactPoints):
		Worker("DatabaseWorker", 900), d_ContactPoints(d_ContactPoints)
	{
	}

	void DatabaseWorker::startupTask(void)
	{
		this->w_Logger << "Verbinding maken met: " << this->d_ContactPoints << ENDL;
		this->d_Connection = std::make_unique<CassandraConnection>(this->d_ContactPoints.c_str());
		this->w_Logger << "Verbinding met database is in stand gebracht !" << ENDL;
	}
}