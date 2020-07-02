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
	void actionHelloInitial(
		BasicActionData &data
	)
	{
		// Performs some checks if the arguments are actually
		// - valid for usage, if not throw an error
		if (data.command.c_Arguments.size() == 0)
			throw SyntaxException("Empty HELO or EHLO is not allowed");
		if (data.command.c_Arguments.size() > 1)
			throw SyntaxException("Only one argument is allowed");

		// Sends the response since everything seems fine to
		// - me, and then we return without any error
		ServerResponse resp(SMTPResponseCommand::SRC_HELO_RESP, data.esmtp, nullptr);
		std::string mess;
		resp.build(mess);
		SMTPSocket::sendString(data.fd, false, mess);
	}

	void actionMailFrom(
		BasicActionData &data,
		Logger& logger,
		std::unique_ptr<CassandraConnection> &database
	)
	{
		// Performs some check if the arguments are there
		// - if not there we throw an error
		if (data.command.c_Arguments.size() == 0)
			throw SyntaxException("Empty MAIL FROM is not allowed");
		else if (data.command.c_Arguments.size() > 1)
			throw SyntaxException("MAIL FROM may only have one single argument");

		// Parses the email address
		try
		{
			EmailAddress addr(data.command.c_Arguments[0]);
			DEBUG_ONLY(logger << DEBUG << "Email from: " << addr.getName() << '<' << addr.getAddress() << '>' << ENDL << CLASSIC);

			// Checks if this is an receive operation or relay operation
			// - by reading the local addresses from the database
			LocalDomain domain;
			domain.getByDomain(addr.getDomain(), database);
		} catch (const std::runtime_error &e)
		{
			throw SyntaxException(e.what());
		}
	}
}