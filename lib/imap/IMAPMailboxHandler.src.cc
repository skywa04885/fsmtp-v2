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

#include "IMAPMailboxHandler.src.h"

namespace FSMTP::IMAP::MAILBOX_HANDLER
{
	/**
	 * Gets the list query type from a string
	 *
	 * @Param {const std::string &} arg0
	 * @Param {const std::string &} arg1
	 * @Return {listQuery}
	 */
	listQuery listQueryFromString(const std::string &arg0, const std::string &arg1)
	{
		// Checks the type, and if we need to go more in depth
		if (arg1[0] == '*')
			return listQuery{ListQueryType::LQT_ALL, 0};
		else if (arg1[0] == '%')
		{
			std::size_t depth = 0;
			std::stringstream stream(arg1);
			std::string token;
			while (std::getline(stream, token, '/'))
				++depth;
			return listQuery{ListQueryType::LQT_LAYERS, depth};
		} else
			return listQuery{ListQueryType::LQT_OTHER};
	}

	/**
	 * Handles the 'LIST' command
	 *
	 * @Param {IMAPClientSocket *} client
	 * @Param {IMAPCommand &} command
	 * @Param {IMAPServerSession &} session
	 * @Param {RedisConnection *} redis
	 * @Param {CassandraConnection *} cassandra
	 * @Return {void}
 	 */
	void list(
		IMAPClientSocket *client,
		IMAPCommand &command,
		IMAPServerSession &session,
		RedisConnection *redis,
		CassandraConnection *cassandra
	)
	{
		// Checks if there are enough arguments
		if (command.c_Args.size() < 2)
			throw IMAPBad("Invalid arguments");


		// Checks if the types are valid
		if ((command.c_Args[0]->n_Type == NT_ATOM || command.c_Args[0]->n_Type == NT_STRING) &&
			(command.c_Args[1]->n_Type == NT_ATOM || command.c_Args[1]->n_Type == NT_STRING))
		{
			throw IMAPBad("Arguments invalid, required ATOM/STRING ATOM/STRING");
		}

		// Parses the arguments, and removes the quotes
		std::string arg0 = command.c_Args[0]->getString();
		std::string arg1 = command.c_Args[1]->getString();
		removeStringQuotes(arg0);
		removeStringQuotes(arg1);

		// Gets all the mailboxes
		std::vector<Mailbox> mailboxes = Mailbox::gatherAll(
			cassandra,
			session.s_Account.a_Bucket,
			session.s_Account.a_Domain,
			session.s_Account.a_UUID,
			false
		);

		// Checks what the client tries to receive, and
		// - prints it to the debug console
		listQuery query = listQueryFromString(arg0, arg1);

		// Performs switch on the actions
		switch (query.l_Type)
		{
			case ListQueryType::LQT_ALL:
			{
				client->sendString(IMAPResponse::buildList(IMAPCommandType::ICT_LIST,
					command.c_Index, mailboxes));
				break;
			}
			case ListQueryType::LQT_OTHER:
			{
				client->sendString(IMAPResponse::buildCompleted(
					command.c_Index, IMAPCommandType::ICT_LIST));
			}
			default: throw std::runtime_error(EXCEPT_DEBUG("Query type not implemented"));
		}
	}

	/**
	 * Handles the 'LSUB' command
	 *
	 * @Param {IMAPClientSocket *} client
	 * @Param {IMAPCommand &} command
	 * @Param {IMAPServerSession &} session
	 * @Param {RedisConnection *} redis
	 * @Param {CassandraConnection *} cassandra
	 * @Return {void}
 	 */
	void lsub(
		IMAPClientSocket *client,
		IMAPCommand &command,
		IMAPServerSession &session,
		RedisConnection *redis,
		CassandraConnection *cassandra
	)
	{
		if (command.c_Args.size() < 2)
			throw IMAPBad("Arguments argument count, required 2");


		// Checks if the types are valid
		if ((command.c_Args[0]->n_Type == NT_ATOM || command.c_Args[0]->n_Type == NT_STRING) &&
			(command.c_Args[1]->n_Type == NT_ATOM || command.c_Args[1]->n_Type == NT_STRING))
		{
			throw IMAPBad("Arguments invalid, required ATOM/STRING ATOM/STRING");
		}

		// Parses the arguments, and removes the quotes
		std::string arg0 = command.c_Args[0]->getString();
		std::string arg1 = command.c_Args[1]->getString();
		removeStringQuotes(arg0);
		removeStringQuotes(arg1);

		// Gets all the mail boxes
		std::vector<Mailbox> mailboxes = Mailbox::gatherAll(
			cassandra,
			session.s_Account.a_Bucket,
			session.s_Account.a_Domain,
			session.s_Account.a_UUID,
			true
		);

		// Checks what the client tries to receive, and
		// - prints it to the debug console
		listQuery query = listQueryFromString(arg0, arg1);

		// Performs switch on the actions
		switch (query.l_Type)
		{
			case ListQueryType::LQT_ALL:
			{
				client->sendString(IMAPResponse::buildList(IMAPCommandType::ICT_LSUB,
					command.c_Index, mailboxes));
				break;
			}
			default: throw std::runtime_error(EXCEPT_DEBUG("Query type not implemented"));
		}
	}

	/**
	 * Handles the 'SELECT' command
	 *
	 * @Param {IMAPClientSocket *} client
	 * @Param {IMAPCommand &} command
	 * @Param {IMAPServerSession &} session
	 * @Param {RedisConnection *} redis
	 * @Param {CassandraConnection *} cassandra
	 * @Return {void}
 	 */
	void select(
		IMAPClientSocket *client,
		IMAPCommand &command,
		IMAPServerSession &session,
		RedisConnection *redis,
		CassandraConnection *cassandra
	)
	{
		// Checks the argument size
		if (command.c_Args.size() < 1)
			throw IMAPBad("Invalid argument count, required 1");

		// Checks if the arguments are valid
		if (
			command.c_Args[0]->n_Type != NT_STRING &&
			command.c_Args[0]->n_Type != NT_ATOM
		)
		{
			throw IMAPBad("Invalid argument type, required STRING/ATOM");
		}

		std::string mailboxName = command.c_Args[0]->getString();
		removeStringQuotes(mailboxName);

		// Query's for the specified mailbox
		MailboxStatus status;
		try
		{
			status = MailboxStatus::get(
				redis, cassandra, session.s_Account.a_Bucket,
				session.s_Account.a_Domain, session.s_Account.a_UUID,
				mailboxName
			);
		} catch (const EmptyQuery &e)
		{
			throw IMAPNo("Can't selekt mailbox");
		}

		// Updates the session
		session.s_SelectedMailbox = mailboxName;
		session.s_State = ServerSessionState::SST_SEL;

		// Writes the response
		client->sendString(IMAPResponse::buildSelectInformation(
			command.c_Index, status));
	}
}