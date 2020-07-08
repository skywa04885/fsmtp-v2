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

namespace FSMTP::Server
{
	void SMTPServerSession::setSSLFlag(void)
	{
		this->s_Flags |= _SMTP_SERV_SESSION_SSL_FLAG;
	}

	bool SMTPServerSession::getSSLFlag(void)
	{
		if (BINARY_COMPARE(this->s_Flags, _SMTP_SERV_SESSION_SSL_FLAG))
			return true;
		else return false;
	}
}