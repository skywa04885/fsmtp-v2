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

std::mutex _emailStorageMutex;
std::vector<FullEmail> _emailStorageQueue = {};

namespace FSMTP::Workers
{
	/**
	 * Default constructor for the database worker
	 *
	 * @Param {const std::string &} d_ContactPoints
	 * @Return {void}
	 */
	DatabaseWorker::DatabaseWorker(const std::string &d_ContactPoints):
		Worker("DatabaseWorker", 900), d_ContactPoints(d_ContactPoints)
	{
	}

	/**
	 * The startup action of the worker
	 *
	 * @Param {void}
	 * @Return {void *} u
	 */
	void DatabaseWorker::startupTask(void)
	{
		this->w_Logger << "Verbinding maken met: " << this->d_ContactPoints << ENDL;
		this->d_Connection = std::make_unique<CassandraConnection>(this->d_ContactPoints.c_str());
		this->w_Logger << "Verbinding met database is in stand gebracht !" << ENDL;
	}

	/**
	 * The action that gets performed at interval
	 *
	 * @Param {void *} u
	 */	
	void DatabaseWorker::action(void *u)
	{
		// ==================================
		// Starts storing the emails
		//
		// Checks if there are any emails
		// - and if so, send them
		// ==================================

		// Checks if there are any emails 
		// - which needs to be stored
		if (_emailStorageQueue.size() <= 0) return;

		// Starts looping over the emails and storing them,
		// TODO: Add batch
		_emailStorageMutex.lock();
		this->w_Logger << "Started storing " << _emailStorageQueue.size() << " emails !" << ENDL;
		for (FullEmail& email : _emailStorageQueue)
			email.save(this->d_Connection);
		this->w_Logger << "Stored " << _emailStorageQueue.size() << " emails, clearing vector" << ENDL;
		_emailStorageQueue.clear();
		_emailStorageMutex.unlock();
	}
}