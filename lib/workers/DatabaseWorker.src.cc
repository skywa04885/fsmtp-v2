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
std::vector<std::pair<std::string, FullEmail>> _emailStorageQueue;
extern Json::Value _config;

namespace FSMTP::Workers
{
	/**
	 * Default constructor for the database worker
	 *
	 * @Param {void{}
	 * @Return {void}
	 */
	DatabaseWorker::DatabaseWorker(void):
		Worker("DatabaseWorker", 900)
	{}

	/**
	 * The startup action of the worker
	 *
	 * @Param {void}
	 * @Return {void *} u
	 */
	void DatabaseWorker::startupTask(void)
	{
		this->d_Cassandra = std::make_unique<CassandraConnection>(_config["database"]["cassandra_hosts"].asCString());
		this->w_Logger << "Verbinding met Cassandra is in stand gebracht !" << ENDL;

		this->d_Redis = std::make_unique<RedisConnection>(_config["database"]["redis_hosts"].asCString(), _config["database"]["redis_port"].asInt());
		this->w_Logger << "Verbinding met Redis is in stand gebracht !" << ENDL;
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
		for (std::pair<std::string, FullEmail>& dataPair : _emailStorageQueue)
		{
			// Generates the stuff like the shortcuts and raw messages
			EmailShortcut shortcut;
			shortcut.e_Domain = dataPair.second.e_OwnersDomain;
			shortcut.e_Subject = dataPair.second.e_Subject;
			shortcut.e_OwnersUUID = dataPair.second.e_OwnersUUID;
			shortcut.e_EmailUUID = dataPair.second.e_EmailUUID;
			shortcut.e_Bucket = dataPair.second.e_Bucket;
			shortcut.e_SizeOctets = dataPair.first.size();

			RawEmail raw;
			raw.e_Bucket = dataPair.second.e_Bucket;
			raw.e_Domain = dataPair.second.e_OwnersDomain;
			raw.e_OwnersUUID = dataPair.second.e_OwnersUUID;
			raw.e_EmailUUID = dataPair.second.e_EmailUUID;
			raw.e_Content = std::move(dataPair.first);

			// Finds an text section if not we will have no preview
			for (const EmailBodySection &section : dataPair.second.e_BodySections)
			{
				if (section.e_Type == EmailContentType::ECT_TEXT_PLAIN)
				{
					shortcut.e_Preview = section.e_Content.substr(
						0,
						(section.e_Content.size() > 255 ? 255 : section.e_Content.size())
					);
				}
			}

			// Checks in which folder the message belongs
			if (dataPair.second.e_Type == EmailType::ET_INCOMMING)
			{
				shortcut.e_Mailbox = "INBOX";
			} else if (dataPair.second.e_Type == EmailType::ET_INCOMMING_SPAM)
			{
				shortcut.e_Mailbox = "INBOX.Spam";
			} else if (
				dataPair.second.e_Type == EmailType::ET_OUTGOING ||
				dataPair.second.e_Type == EmailType::ET_RELAY_OUTGOING
			)
			{
				shortcut.e_Mailbox = "INBOX.Sent";
			}

			// Updates the folder
			shortcut.e_UID = MailboxStatus::addOneMessage(
				this->d_Redis.get(),
				this->d_Cassandra.get(),
				dataPair.second.e_OwnersBucket,
				dataPair.second.e_OwnersDomain,
				dataPair.second.e_OwnersUUID,
				shortcut.e_Mailbox
			);

			// Stores the shit
			raw.save(this->d_Cassandra.get());
			shortcut.save(this->d_Cassandra.get());
		}
		this->w_Logger << "Stored " << _emailStorageQueue.size() << " emails, clearing vector" << ENDL;
		_emailStorageQueue.clear();
		_emailStorageMutex.unlock();
	}
}
