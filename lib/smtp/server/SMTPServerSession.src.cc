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

using namespace FSMTP::Server;

SMTPServerSession::SMTPServerSession():
	s_Flags(0x0), s_PerformedActions(0x0), s_PossSpam(false)
{}

void SMTPServerSession::setFlag(int64_t mask) {
	this->s_Flags |= mask;
}

bool SMTPServerSession::getFlag(int64_t mask) {
	if (BINARY_COMPARE(this->s_Flags, mask)) {
		return true;
	} else {
		return false;
	}
}

void SMTPServerSession::setAction(int64_t mask) {
	this->s_PerformedActions |= mask;
}

void SMTPServerSession::clearAction(int64_t mask) {
	this->s_PerformedActions &= ~mask;
}

bool SMTPServerSession::getAction(int64_t mask) {
	if (BINARY_COMPARE(this->s_PerformedActions, mask)) {
		return true;
	} else {
		return false;
	}
}
