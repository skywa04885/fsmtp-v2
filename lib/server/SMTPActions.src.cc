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
	/**
	 * Handles the "HELO" / "EHLO" command of the SMTP Server
	 * 
	 * @Param {BasicActionData &data} data
	 * @Return {void}
	 */
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
		// ServerResponse resp(SMTPResponseCommand::SRC_HELO_RESP, data.esmtp, nullptr);
		// std::string mess;
		// resp.build(mess);
		// SMTPSocket::sendString(data.fd, nullptr, false, mess);
	}

	/**
	 * Handles "HELO" / "EHLO" after StartTLS command
	 *
	 * @Param {BasicActionData &} data
	 * @Return {void}
	 */
	void actionHelloAfterStartTLS(
		BasicActionData &data,
		std::vector<SMTPServiceFunction> *services,
		SMTPServerSession &session
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
		// ServerResponse resp(SMTPResponseCommand::SRC_HELO_RESP, data.esmtp, services);
		// std::string mess;
		// resp.build(mess);
		// SMTPSocket::sendString(data.fd, data.ssl, session.getSSLFlag(), mess);
	}

	/**
	 * Handles the "MAIL FROM" command of the SMTP Server
	 *
	 * @Param {BasicActionData &} data
	 * @Param {Logger &} logger
	 * @Param {std::unique_ptr<CassandraConnection> &} database
	 * @Param {SMTPServerSession &} session
	 * @Return {void}
	 */
	void actionMailFrom(
		BasicActionData &data,
		Logger& logger,
		RedisConnection *redis,
		SMTPServerSession &session
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
			// Parses the address and then
			// - gets the info such as username and domain
			session.s_TransportMessage.e_TransportFrom.parse(data.command.c_Arguments[0]);
			std::string tDomain;
			session.s_TransportMessage.e_TransportFrom.getDomain(tDomain);
			DEBUG_ONLY(logger << DEBUG << "Email from: " << session.s_TransportMessage.e_TransportFrom.e_Name << '<' << session.s_TransportMessage.e_TransportFrom.e_Address << '>' << ENDL << CLASSIC);

			// Checks if this is an receive operation or relay operation
			// - by reading the local addresses from the database
			LocalDomain domain;
			try {
				// Performs the query and checks if we're
				// - in controll of this domain, after that
				// - we check if the address is here, if they
				// - both are true, then we set the relay flag
				domain.findRedis(tDomain, redis);

				std::string username;
				session.s_TransportMessage.e_TransportFrom.getUsername(username);
				session.s_SendingAccount = AccountShortcut::findRedis(
					redis,
					tDomain,
					username
				);

				session.s_Flags |= _SMTP_SERV_SESSION_RELAY_FLAG;
			}
			catch (const EmptyQuery &e)
			{
				session.s_Flags &= ~_SMTP_SERV_SESSION_RELAY_FLAG;
			}
		} catch (const std::runtime_error &e)
		{
			throw SyntaxException(e.what());
		}

		// Sends the response which states that the client
		// - may proceed with sending the email
		// ServerResponse resp(SMTPResponseCommand::SRC_PROCEED, data.esmtp, nullptr);
		// std::string mess;
		// resp.build(mess);
		// SMTPSocket::sendString(data.fd, data.ssl, session.getSSLFlag(), mess);
	}

	/**
	 * Handles the "RCPT TO" command of the SMTP Server
	 *
	 * @Param {BasicActionData &} data
	 * @Param {Logger&} logger
	 * @Param {std::unique_ptr<CassandraConnection> &} database
	 * @Param {SMTPServerSession &} session
	 * @Return {void}
	 */
	void actionRcptTo(
		BasicActionData &data,
		Logger& logger,
		RedisConnection *redis,
		SMTPServerSession &session
	)
	{
		// Performs some check if the arguments are there
		// - if not there we throw an error
		if (data.command.c_Arguments.size() == 0)
			throw SyntaxException("Empty RCPT TO is not allowed");
		else if (data.command.c_Arguments.size() > 1)
			throw SyntaxException("RCPT TO may only have one single argument");


		// Parses the email address
		try
		{
			// Parses the address and then
			// - gets the info such as username and domain
			session.s_TransportMessage.e_TransportTo.parse(data.command.c_Arguments[0]);
			std::string tDomain;
			session.s_TransportMessage.e_TransportTo.getDomain(tDomain);
			DEBUG_ONLY(logger << DEBUG << "Email to: " << session.s_TransportMessage.e_TransportTo.e_Name << '<' << session.s_TransportMessage.e_TransportTo.e_Address << '>' << ENDL << CLASSIC);

			// Checks if this is an receive operation or relay operation
			// - by reading the local addresses from the database
			LocalDomain domain;
			try {
				// Checks if the domain is is in the database
				domain.findRedis(tDomain, redis);

				// Checks if the user is in the database,
				// - if not we cannot send the email, except
				// - we're relaying
				std::string username;
				session.s_TransportMessage.e_TransportTo.getUsername(username);
				session.s_ReceivingAccount = AccountShortcut::findRedis(
					redis,
					tDomain,
					username
				);

				#ifdef _SMTP_DEBUG
				char uuidString[64];
				cass_uuid_string(session.s_ReceivingAccount.a_UUID, uuidString);
				logger << DEBUG << "Target user found locally: " << uuidString << "@" << session.s_ReceivingAccount.a_Bucket  << ENDL << CLASSIC;
				#endif

				// If we're relaying, and the domain is in
				// - the database, we want to set the relay
				// - to local flag
				if ((session.s_Flags & _SMTP_SERV_SESSION_RELAY_FLAG) == _SMTP_SERV_SESSION_RELAY_FLAG)
				{
					session.s_Flags |= _SMTP_SERV_SESSION_RELAY_TO_LOCAL;
				}
			}
			catch (const EmptyQuery &e)
			{
				if ((session.s_Flags & _SMTP_SERV_SESSION_RELAY_FLAG) != _SMTP_SERV_SESSION_RELAY_FLAG)
				{
					// Sends the error message
					// ServerResponse resp(SMTPResponseCommand::SRC_BAD_EMAIL_ADDRESS, data.esmtp, nullptr);
					// std::string mess;
					// resp.build(mess);
					// SMTPSocket::sendString(data.fd, data.ssl, session.getSSLFlag(), mess);

					// Throws the fatal exception
					std::string message = "Gebruiker of domain niet gevonden in onze database: ";
					message += e.what();
					throw FatalException(message);
				}
			}
		} catch (const std::runtime_error &e)
		{
			throw SyntaxException(e.what());
		}

		// Sends the response which states that the client
		// - may proceed with sending the email
		// ServerResponse resp(SMTPResponseCommand::SRC_PROCEED, data.esmtp, nullptr);
		// std::string mess;
		// resp.build(mess);
		// SMTPSocket::sendString(data.fd, data.ssl, session.getSSLFlag(), mess);
	}
}