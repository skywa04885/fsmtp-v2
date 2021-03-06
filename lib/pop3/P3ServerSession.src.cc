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

#include "P3ServerSession.src.h"

namespace FSMTP::POP3
{
	P3ServerSession::P3ServerSession():
		s_Flags(0x0), s_Actions(0x0)
	{}

	void P3ServerSession::setFlag(int64_t mask) {
		this->s_Flags |= mask;
	}

	bool P3ServerSession::getFlag(int64_t mask) {
		if (BINARY_COMPARE(this->s_Flags, mask)) {
			return true;
		} else {
			return false;
		}
	}

	void P3ServerSession::setAction(int64_t mask) {
		this->s_Actions |= mask;
	}

	bool P3ServerSession::getAction(int64_t mask) {
		if (BINARY_COMPARE(this->s_Actions, mask)) {
			return true;
		} else {
			return false;
		}
	}
}