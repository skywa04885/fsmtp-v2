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

#pragma once

#include "SMTPActions.src.h"

namespace FSMTP::Server::Actions
{
	void actionHelloInitial(const ClientCommand &command, int32_t &fd, struct sockaddr_in &sAddr, const bool &ssl, const bool &esmtp)
	{
		// Performs some checks if the arguments are actually
		// - valid for usage, if not throw an error
		if (command.c_Arguments.size() == 0)
			throw std::runtime_error("Empty HELO or EHLO is not allowed");
		if (command.c_Arguments.size() > 1)
			throw std::runtime_error("Only one argument is allowed");

		// Sends the response since everything seems fine to
		// - me, and then we return without any error
		ServerResponse resp(SMTPResponseCommand::SRC_HELO_RESP, esmtp, nullptr);
		std::string mess;
		resp.build(mess);
		SMTPSocket::sendString(fd, false, mess);
	}
}