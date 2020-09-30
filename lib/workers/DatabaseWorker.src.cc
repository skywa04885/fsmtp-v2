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

mutex databaseMutex;
deque<shared_ptr<SMTPServerSession>> databaseQueue;

namespace FSMTP::Workers
{
	DatabaseWorker::DatabaseWorker(void):
		Worker("FSMTP-V2/STORAGE", 900)
	{}

	void DatabaseWorker::startupTask(void) {
		auto &logger = this->w_Logger;

		this->d_Cassandra = Global::getCassandra();
		logger << _BASH_SUCCESS_MARK << "Connected to cassandra" << ENDL;

		this->d_Redis = Global::getRedis();
		logger << _BASH_SUCCESS_MARK << "Connected to redis" << ENDL;
	}

	void DatabaseWorker::push(shared_ptr<SMTPServerSession> session) {
		databaseMutex.lock();
		databaseQueue.push_back(session);
		databaseMutex.unlock();
	}

	void DatabaseWorker::action(void *u) {
		auto *cassandra = this->d_Cassandra.get();
		auto *redis = this->d_Redis.get();
		auto &logger = this->w_Logger;

		for (;;) {
			// ============================
			// Gets the front of the queue
			// ============================

			databaseMutex.lock(); // Locks the database mutex

			// Checks if there are any emails we have to process
			//  else just exit the worker action
			if (databaseQueue.size() <= 0) {
				databaseMutex.unlock();
				break;
			}

			// Gets the last task from the queue, and pops it from it
			//  so we will not have to process it again
			shared_ptr<SMTPServerSession> session = databaseQueue.back();
			databaseQueue.pop_back();

			databaseMutex.unlock(); // Unlocks the database mutex

			// ============================
			// Stores the messages
			// ============================

			// Loops over the storage tasks, while we will
			//  start storing the raw messages with the
			//  shortcuts to them
			auto storageTasks = session->getStorageTasks();
			DEBUG_ONLY(logger << DEBUG << "Performing " << storageTasks.size() << " storage tasks .." << ENDL << CLASSIC);
			for_each(storageTasks.begin(), storageTasks.end(), [&](const SMTPServerStorageTask &task) {
				CassUuid messageUUID;
				int64_t messageBucket;
				int32_t messageUID;
				string messageMailbox;

				// Checks the current storage target type, and to which mailbox name
				//  it refers, after which we will set the mailbox string
				switch (task.target) {
					case SMTPServerStorageTarget::StorageTargetIncomming:
						messageMailbox = "INBOX"; break;
					case SMTPServerStorageTarget::StorageTargetSent:
						messageMailbox = "INBOX.Sent"; break;
					case SMTPServerStorageTarget::StorageTargetSpam:
						messageMailbox = "INBOX.Spam"; break;
					default:
						messageMailbox = "INBOX"; break;
				}

				// Increments the UID holder, and gets the UID for the current email
				//  the UID holder keeps track of the unique id's
				try {
					messageUID = UIDHolder::getAndIncrement(cassandra, redis, 
						task.account.getBucket(), task.account.getDomain(), task.account.getUUID());
				} catch (const runtime_error &e) {
					logger << ERROR << "Could not update/read UID, runtime error: " << e.what() << ENDL << CLASSIC;
					return;
				} catch (const DatabaseException &e) {
					logger << ERROR << "Could not update/read UID, database exception: " << e.what() << ENDL << CLASSIC;
					return;
				} catch (...) {
					logger << ERROR << "Could not update/read UID, error unknown" << ENDL << CLASSIC;
					return;
				}

				// Increments the number of emails for the target mailbox
				try {
					MailboxStatus::addOneMessage(redis, cassandra, task.account.getBucket(),
						task.account.getDomain(), task.account.getUUID(),
						messageMailbox);
				} catch (const runtime_error &e) {
					logger << ERROR << "Could not update mailbox, runtime error: " << e.what() << ENDL << CLASSIC;
					return;
				} catch (const DatabaseException &e) {
					logger << ERROR << "Could not update mailbox, database exception: " << e.what() << ENDL << CLASSIC;
					return;
				} catch (...) {
					logger << ERROR << "Could not update mailbox, error unknown" << ENDL << CLASSIC;
					return;
				}

				// Generates the bucket and TimeUUID for the current email
				//  these will be used to quickly access each specified message
				messageBucket = FullEmail::getBucket();
				messageUUID = FullEmail::generateMessageUUID();

				// Prints the message that we attempt to save one message
				//  to the specified mailbox
				DEBUG_ONLY(logger << DEBUG << "Saving message to mailbox: '" << 
					messageMailbox << "', for user: '" << task.account.getUsername() << '@'
					<< task.account.getDomain() << "'" << ENDL << CLASSIC);

				// Creates the email shortcut, this will be used to quickly
				//  list all the emails in the databse, without the real
				//  data inside of it
				EmailShortcut shortcut(task.account.getDomain(), session->getSubject(), 
					session->getSnippet(), session->getTransportFrom().toString(), 
					task.account.a_UUID, messageUUID, messageUID, 0x0, messageBucket,
					messageMailbox, session->raw().size());

				RawEmail raw(messageBucket, task.account.getDomain(), task.account.getUUID(),
					messageUUID, session->raw());

				// Saves the shortcut and the raw email to the database, if any error
				//  occures we will just log the error, and proceed with the next one
				try {
					shortcut.save(cassandra);
					raw.save(cassandra);
				} catch (const runtime_error &e) {
					logger << ERROR << "Could not store message, runtime error: " << e.what() << ENDL << CLASSIC;
				} catch (const DatabaseException &e) {
					logger << ERROR << "Could not store message, database exception: " << e.what() << ENDL << CLASSIC;
				} catch (...) {
					logger << ERROR << "Could not store message, unknown error" << ENDL << CLASSIC;
				}

				// Prints the final message to indicate that the message has been stored
				//  and no errors occured
				DEBUG_ONLY(logger << DEBUG << "Saved message to mailbox: '" << 
					messageMailbox << "', for user: '" << task.account.getUsername() << '@'
					<< task.account.getDomain() << "'" << ENDL << CLASSIC);
			});
		}
	}
}
