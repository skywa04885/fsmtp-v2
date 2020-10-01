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

#include "SMTPServerSession.src.h"

namespace FSMTP::Server {
	SMTPServerSession::SMTPServerSession():
		m_Flags(0x0), m_PerformedActions(0x0), m_PossibleSpam(false)
	{}

	void SMTPServerSession::setFlag(int64_t mask) {
		this->m_Flags |= mask;
	}

	bool SMTPServerSession::getFlag(int64_t mask) {
		if (BINARY_COMPARE(this->m_Flags, mask)) return true;
		else return false;
	}

	void SMTPServerSession::setAction(int64_t mask) {
		this->m_PerformedActions |= mask;
	}

	void SMTPServerSession::clearAction(int64_t mask) {
		this->m_PerformedActions &= ~mask;
	}

	bool SMTPServerSession::getAction(int64_t mask) {
		if (BINARY_COMPARE(this->m_PerformedActions, mask)) return true;
		else return false;
	}

	SMTPServerSession &SMTPServerSession::storageTasksToSpam() {
		for_each(this->m_StorageTasks.begin(), this->m_StorageTasks.end(), [](SMTPServerStorageTask &task) {
			if (task.target == SMTPServerStorageTarget::StorageTargetIncomming)
				task.target = SMTPServerStorageTarget::StorageTargetSpam;
		});
		
		return *this;
	}

	SMTPServerSession &SMTPServerSession::removeSentTasks() {
		for (auto it = this->m_StorageTasks.begin(); it != this->m_StorageTasks.end(); ++it)
			if (it->target == SMTPServerStorageTarget::StorageTargetSent) {
				this->m_StorageTasks.erase(it);
				break;
			}
		
		return *this;
	}

	SMTPServerSession &SMTPServerSession::addStorageTask(const SMTPServerStorageTask &task) {
		this->m_StorageTasks.push_back(task);
		return *this;
	}

	SMTPServerSession &SMTPServerSession::addRelayTask(const SMTPServerRelayTask &task) {
		this->m_RelayTasks.push_back(task);
		return *this;
	}

	SMTPServerSession &SMTPServerSession::addTransportTo(const EmailAddress &address) {
		this->m_TransportTo.push_back(address);
		return *this;
	}

	const vector<SMTPServerStorageTask> &SMTPServerSession::getStorageTasks() {
		return this->m_StorageTasks;
	}

	const vector<SMTPServerRelayTask> &SMTPServerSession::getRelayTasks() {
		return this->m_RelayTasks;
	}

	const vector<EmailAddress> &SMTPServerSession::getTransportTo() {
		return this->m_TransportTo;
	}

	SMTPServerSession &SMTPServerSession::setTransformFrom(const EmailAddress &address) {
		this->m_TransportFrom = address;
		return *this;
	}

	const EmailAddress& SMTPServerSession::getTransportFrom() {
		return this->m_TransportFrom;
	}

	string &SMTPServerSession::raw() {
		return this->m_Raw;
	}

	XFannst::XFannstFlags &SMTPServerSession::xfannst() {
		return this->m_XFannstFlags;
	}

	SMTPServerSession &SMTPServerSession::setMessageID(const string &id) {
		this->m_MessageID = id;
		return *this;
	}

	SMTPServerSession &SMTPServerSession::setSubject(const string &subject) {
		this->m_Subject = subject;
		return *this;
	}

	SMTPServerSession &SMTPServerSession::setSnippet(const string &snippet) {
		this->m_Snippet = snippet;
		return *this;
	}

	SMTPServerSession &SMTPServerSession::generateSnippet(const string &raw) {
		if (raw.length() > 128) this->m_Snippet = raw.substr(0, 128);
		else this->m_Snippet = raw;
		return *this;
	}

	const string &SMTPServerSession::getMessageID() {
		return this->m_MessageID;
	}

	const string &SMTPServerSession::getSubject() {
		return this->m_Subject;
	}

	const string &SMTPServerSession::getSnippet() {
		return this->m_Snippet;
	}

	SMTPServerSession &SMTPServerSession::setPossibleSpam(bool v) {
		this->m_PossibleSpam = v;
	}

	bool SMTPServerSession::getPossibleSpam() {
		return this->m_PossibleSpam;
	}

	SMTPServerSession::~SMTPServerSession() = default;
}