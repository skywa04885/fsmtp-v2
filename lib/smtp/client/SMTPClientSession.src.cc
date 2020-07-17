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

#include "SMTPClientSession.src.h"

namespace FSMTP::Mailer::Client
{
	/**
	 * Default constructor, sets the flags blank
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	SMTPClientSession::SMTPClientSession():
		s_PerformedActions(0x0), s_SessionFlags(0x0)
	{}

	/**
	 * Sets an flag
	 *
	 * @Param {int64_t} mask
	 * @Return {void}
	 */
	void SMTPClientSession::setFlag(int64_t mask)
	{
		this->s_SessionFlags |= mask;
	}
	
	/**
	 * Gets an flag as bool
	 *
	 * @Param {int64_t} mask
	 * @Return {bool}
	 */
	bool SMTPClientSession::getFlag(int64_t mask)
	{
		if (BINARY_COMPARE(this->s_SessionFlags, mask))
			return true;
		else
			return false;
	}

	/**
	 * Sets an action
	 *
	 * @Param {int64_t} mask
	 * @Return {void}
	 */
	void SMTPClientSession::setAction(int64_t mask)
	{
		this->s_PerformedActions |= mask;
	}
	
	/**
	 * Gets an action as bool
	 *
	 * @Param {int64_t} mask
	 * @Return {bool}
	 */
	bool SMTPClientSession::getAction(int64_t mask)
	{
		if (BINARY_COMPARE(this->s_PerformedActions, mask))
			return true;
		else
			return false;
	}

	/**
	 * Gets an action and returns the
	 * - bool value, and sets it
	 *
	 * @Param {int64_t mask}
	 * @Return {void}
	 */
	bool SMTPClientSession::getActionSet(int64_t mask)
	{
		if (BINARY_COMPARE(this->s_PerformedActions, mask))
			return true;
		else
		{
			this->setAction(mask);
			return false;
		}
	}

	/**
	 * Clears an action
	 *
	 * @Param {int64_t} mask
	 * @Return {void}
	 */
	void SMTPClientSession::clearAction(int64_t mask)
	{
		this->s_PerformedActions &= ~mask;
	}
}