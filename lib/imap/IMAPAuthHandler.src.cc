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

#include "IMAPAuthHandler.src.h"

namespace FSMTP::IMAP::AUTH_HANDLER
{
	/**
	 * Handles the 'LOGIN' command
	 *
	 * @Param {IMAPClientSocket *} client
	 * @Param {IMAPCommand &} command
	 * @Param {IMAPServerSession &} session
 	 * @Param {RedisConnection *} redis
	 * @Param {CassandraConnection *} cassandra
	 * @Return {void}
 	 */
	void login(
		IMAPClientSocket *client,
		IMAPCommand &command,
		IMAPServerSession &session,
		RedisConnection *redis,
		CassandraConnection *cassandra
	)
	{
		// Checks if the parameter count is correct, if not
		// - throw bad error
		if (command.c_Args.size() < 2)
			throw IMAPBad("Arguments invalid");

		// Gets the username and password
		std::string &user = command.c_Args[0];
		std::string &pass = command.c_Args[1];

		// Checks if the email address contains domain,
		// - if not append one
		if (user.find_first_of('@') == std::string::npos)
			user += _SMTP_DEF_DOMAIN;

		// Parses the email address, and throws error
		// - if the username is invalid
		EmailAddress address;
		try
		{
			address.parse(user);
		} catch (const std::runtime_error &e)
		{
			throw IMAPBad("Username rejected: invalid address");
		}

		// Checks if the domain is on our server, if not
		// - reject username
		try
		{
			LocalDomain domain = LocalDomain::findRedis(address.getDomain(), redis);
		} catch (const EmptyQuery &e)
		{
			throw IMAPNo("Username rejected: domain not local");
		}

		// Checks if the username is local,
		// - if not reject entry
		try
		{
			session.s_Account = AccountShortcut::findRedis(
				redis,
				address.getDomain(),
				address.getUsername()
			);
		} catch (const EmptyQuery &e)
		{
			throw IMAPNo("Username rejected: user not local");
		}

		// Gets the users public key, and password, so we can check
		// - if the password is correct
		std::string validPassword, publicKey;
		try
		{
			std::tie(validPassword, publicKey) = Account::getPassAndPublicKey(
				cassandra,
				address.getDomain(),
				session.s_Account.a_Bucket, 
				session.s_Account.a_UUID
			);
		} catch (const EmptyQuery &e)
		{
			throw IMAPNo("Username rejected: user deleted");
		}

		// Compares the supplied password with the one from the database,
		// - if no match throw no, else we just set the auth flag
		if (!passwordVerify(pass, validPassword))
		{
			session.clearFlag(_IMAP_FLAG_LOGGED_IN);
			throw IMAPNo("Password rejected");
		}
		session.setFlag(_IMAP_FLAG_LOGGED_IN);

		// Writes the success response
		client->sendResponse(
			IRS_TLC, command.c_Index,
			IMAPResponseType::IRT_LOGIN_SUCCESS,
			IMAPResponsePrefixType::IPT_OK,
			nullptr
		);
	}
}