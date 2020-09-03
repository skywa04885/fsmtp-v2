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

#ifndef _LIB_SMTP_SERVER_SMTPSU_H
#define _LIB_SMTP_SERVER_SMTPSU_H

#include "../../default.h"
#include "../../general/Logger.src.h"
#include "../../dns/SPF.src.h"
#include "../../networking/Address.src.h"

using namespace FSMTP::Networking;

namespace FSMTP::Server::SU {
	/**
	 * Checks if an hostname is in the SuperUser list
	 *  this will allow to send it from fannst without
	 *  authenticating.
	 */
	bool checkSU(const string &hostname, const string &checkAddr);

	/**
	 * Gets the MX Addresses for an specific domain, this
	 *  may be used to validate an server.
	 */
	vector<string> getMXAddresses(const string &hostname);

	/**
	 * Gets all the A Records for a specific domain, this
	 *  may be used to validate an server.
	 */
	vector<string> getARecordAddresses(const string &hostname);
}

#endif
