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
vector<shared_ptr<SMTPServerSession>> databaseQueue;

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
		auto *cass = this->d_Cassandra.get();
		auto *redis = this->d_Redis.get();
		auto &logger = this->w_Logger;

		for (;;) {
			// ============================
			// Gets the front of the queue
			// ============================

			databaseMutex.lock(); // Locks the database mutex

			// Checks if there are any emails we have to process
			//  else just exit the worker action
			if (databaseQueue.size() <= 0) break;

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
				messageUID = UIDHolder::getAndIncrement(cass, redis, 
					task.account.getBucket(), task.account.getDomain(), task.account.getUUID());

				// Generates the bucket and TimeUUID for the current email
				//  these will be used to quickly access each specified message
				messageBucket = FullEmail::getBucket();
				messageUUID = FullEmail::generateMessageUUID();

				// Creates the email shortcut, this will be used to quickly
				//  list all the emails in the databse, without the real
				//  data inside of it
				EmailShortcut shortcut(task.account.getDomain(), session->getSubject(), 
					session->getSnippet(), session->getTransportFrom().toString(), 
					task.account.a_UUID, messageUUID, messageUID, 0x0, messageBucket,
					messageMailbox, session->raw().size());

				RawEmail raw(messageBucket, task.account.getDomain(), task.account.getUUID(),
					messageUUID, session->raw());
			});
		}

		// while (databaseQueue.size() > 0) {
		// 	databaseMutex.lock();
		// 	auto session = databaseQueue.front();
		// 	databaseQueue.pop_back();
		// 	databaseMutex.unlock();

		// 	auto &fullEmail = session->m_Message;
		
		// 	// Generates the email shortcut, this will be used
		// 	//  for quick message access in front-end applications

		// 	EmailShortcut shortcut;
		// 	shortcut.e_Subject = fullEmail.e_Subject;
		// 	shortcut.e_SizeOctets = session->s_RawBody.size();
		// 	shortcut.e_From = fullEmail.e_From[0].toString();

		// 	// Generates the raw message, this will be used
		// 	//  for raw access, basically by all the apps, and will parse them.

		// 	RawEmail raw;
		// 	raw.e_Content = session->s_RawBody;

		// 	// Checks if we can generate an preview, if the preview ends up empty
		// 	//  we will put a 'No preview' string

		// 	for (const EmailBodySection &section : fullEmail.e_BodySections) {
		// 		if (section.e_Type == EmailContentType::ECT_TEXT_PLAIN) {
		// 			shortcut.e_Preview = section.e_Content.substr(
		// 				0,
		// 				(section.e_Content.size() > 255 ? 255 : section.e_Content.size())
		// 			);
		// 		}
		// 	}

		// 	if (shortcut.e_Preview.empty()) {
		// 		shortcut.e_Preview = "No preview available";
		// 	}

		// 	auto &username = session->s_SendingAccount.a_Username;
		// 	auto &domain  = session->s_SendingAccount.a_Domain;
		// 	if (username + '@' + domain	== fullEmail.e_TransportFrom.e_Address) {
		// 		shortcut.e_Mailbox = "INBOX.Sent";
		// 	} else if (!session->s_PossSpam) {
		// 		shortcut.e_Mailbox = "INBOX";
		// 	} else {
		// 		shortcut.e_Mailbox = "INBOX.Spam";
		// 	}

		// 	// Updates the folder status, and gets the new
		// 	//  uid, which will be stored in the shortcut. 
		// 	//  the uid is used in POP3

		// 	for_each(session->s_StorageTasks.begin(), session->s_StorageTasks.end(), [&](AccountShortcut &acc) {
		// 		int64_t bucket = FullEmail::getBucket();
		// 		CassUuid uuid = FullEmail::generateMessageUUID();

		// 		shortcut.e_OwnersUUID = acc.a_UUID;
		// 		shortcut.e_Domain = acc.a_Domain;
		// 		shortcut.e_Bucket = bucket;
		// 		shortcut.e_EmailUUID = uuid;

		// 		raw.e_Domain = acc.a_Domain;
		// 		raw.e_OwnersUUID = acc.a_UUID;
		// 		raw.e_Bucket = bucket;
		// 		raw.e_EmailUUID = uuid;
				
		// 		try {
		// 			shortcut.e_UID = UIDHolder::getAndIncrement(
		// 				cass, redis, acc.a_Bucket, acc.a_Domain, acc.a_UUID
		// 			);
		// 			MailboxStatus::addOneMessage(redis, cass, acc.a_Bucket, acc.a_Domain, acc.a_UUID, shortcut.e_Mailbox);

		// 			raw.save(cass);
		// 			shortcut.save(cass);
		// 		} catch (const runtime_error &e) {
		// 			logger << FATAL << "Could not store message: " << e.what() << ENDL << CLASSIC;
		// 		} catch (const EmptyQuery &e) {
		// 			logger << FATAL << "Could not store message: " << e.what() << ENDL << CLASSIC;
		// 		} catch (const DatabaseException &e) {
		// 			logger << FATAL << "Could not store message: " << e.what() << ENDL << CLASSIC;
		// 		}

		// 		DEBUG_ONLY(logger << DEBUG << "Stored 1 message !" << ENDL << CLASSIC);
		// 	});
		// }
	}
}
