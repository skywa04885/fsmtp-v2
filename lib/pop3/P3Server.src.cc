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

#include "P3Server.src.h"

namespace FSMTP::POP3
{
	P3Server::P3Server(const bool secure):
		s_Socket((secure ? _POP3_PORT_SECURE : _POP3_PORT_PLAIN)),
		s_Logger("P3Server", LoggerLevel::INFO)
	{
		// Sets the capabilities
		this->s_Capabilities.push_back(POP3Capability{
			"STLS",
			{}
		});
		this->s_Capabilities.push_back(POP3Capability{
			"USER",
			{}
		});
		this->s_Capabilities.push_back(POP3Capability{
			"EXPIRE",
			{"0"}
		});
		this->s_Capabilities.push_back(POP3Capability{
			"LOGIN-DELAY",
			{"300"}
		});
		this->s_Capabilities.push_back(POP3Capability{
			"TOP",
			{}
		});
		this->s_Capabilities.push_back(POP3Capability{
			"UIDL",
			{}
		});

		// Starts listenign
		this->s_Socket.startListening(
			&this->s_Running,
			&this->s_Run,
			&P3Server::acceptorCallback,
			this
		);
	}

	void P3Server::acceptorCallback(std::unique_ptr<ClientSocket> client, void *u)
	{
		std::string prefix = "P3ServerAcceptor:";
		prefix += inet_ntoa(client->s_SocketAddr.sin_addr);
		Logger logger(prefix, LoggerLevel::INFO);

		P3Server &server = *reinterpret_cast<P3Server *>(u);
		P3ServerSession session;

		// Sends the initial greeting message
		client->sendResponse(true, POP3ResponseType::PRT_GREETING);

		// Attemts to connect to redis
		std::unique_ptr<RedisConnection> redis;
		std::unique_ptr<CassandraConnection> cassandra;
		try {
			redis = std::make_unique<RedisConnection>(_REDIS_CONTACT_POINTS, _REDIS_PORT);
			DEBUG_ONLY(logger << DEBUG << "Verbonden met redis [" << _REDIS_CONTACT_POINTS
				<< ':' << _REDIS_PORT << ']' << ENDL << CLASSIC);
		} catch (const std::runtime_error &e)
		{
			logger << ERROR << "Kon niet verbinden met Redis [" << _REDIS_CONTACT_POINTS
				<< ':' << _REDIS_PORT << "]: " << e.what() << ENDL << CLASSIC;
			goto pop3_session_end;
		}

		// Connects to cassandra
		try {
			cassandra = std::make_unique<CassandraConnection>(_CASSANDRA_DATABASE_CONTACT_POINTS);
			DEBUG_ONLY(logger << DEBUG << "Verbonden met Cassandra ["
				<< _CASSANDRA_DATABASE_CONTACT_POINTS << "]" << ENDL << CLASSIC);
		} catch (const std::runtime_error &e)
		{
			logger << ERROR << "Kon niet verbinden met Cassandra [" << _CASSANDRA_DATABASE_CONTACT_POINTS
				<< "]: " << e.what() << ENDL << CLASSIC;
			goto pop3_session_end;
		}

		for (;;)
		{
			P3Command command;

			try {
				std::string raw = client->readUntillCRLF();
				command.parse(raw);
			} catch (const SocketReadException &e)
			{
				break;
			}

			// Checks how to respond to the command
			try {
				switch (command.c_Type)
				{
					case POP3CommandType::PCT_STLS:
					{
						// Sends the response that the client may switch to start tls, and
						// - then upgrades the socket
						client->sendResponse(true, POP3ResponseType::PRT_STLS_START);
						client->upgrade();

						break;
					}
					case POP3CommandType::PCT_QUIT:
					{
						client->sendResponse(true, PRT_QUIT);
						goto pop3_session_end;
					}
					case POP3CommandType::PCT_PASS:
					{
						// Checks if the command is not empty
						if (command.c_Args.size() < 1)
						{
							client->sendResponse(
								false,
								POP3ResponseType::PRT_SYNTAX_ERROR,
								"",
								nullptr,
								reinterpret_cast<void *>(const_cast<char *>("PASS requires one argument"))
							);
							continue;
						}

						// Gets the user password from the database
						std::string publicKey, password;
						try {
							std::tie(password, publicKey) = Account::getPassAndPublicKey(
								cassandra.get(),
								session.s_Account.a_Domain,
								session.s_Account.a_Bucket,
								session.s_Account.a_UUID
							);
						} catch (const EmptyQuery &e)
						{
							// Constructs the error
							std::string error = "user [";
							error += session.s_Account.a_Username;
							error += '@';
							error += session.s_Account.a_Domain;
							error += "] was not found in cassandra.";

							// Sends the error response
							client->sendResponse(
								false,
								POP3ResponseType::PRT_AUTH_FAIL,
								"",
								nullptr,
								reinterpret_cast<void *>(const_cast<char *>(error.c_str()))
							);
							continue;
						}

						// Compares the passwords to check if it realy is the user
						if (!passwordVerify(command.c_Args[0], password))
						{
							client->sendResponse(
								false,
								POP3ResponseType::PRT_AUTH_FAIL,
								"",
								nullptr,
								reinterpret_cast<void *>(const_cast<char *>("invalid password"))
							);
							continue;
						}

						// Stores the password in the session, and sends
						// - the continue response
						session.s_Pass = command.c_Args[0];
						client->sendResponse(true, POP3ResponseType::PRT_AUTH_SUCCESS);
						break;
					}
					case POP3CommandType::PCT_USER:
					{
						EmailAddress address;

						// Checks if the command is not empty
						if (command.c_Args.size() < 1)
						{
							client->sendResponse(
								false,
								POP3ResponseType::PRT_SYNTAX_ERROR,
								"",
								nullptr,
								reinterpret_cast<void *>(const_cast<char *>("USER requires one argument"))
							);
							continue;
						}

						// Checks if there is an domain, if not
						// - ignore this step and append the default one
						if (command.c_Args[0].find_first_of('@') != std::string::npos)
						{
							address.parse(command.c_Args[0]);

							// Searches the database for the local domain, so we
							// - know if the users domain is on this server
							try {
								LocalDomain domain = LocalDomain::findRedis(address.getDomain(), redis.get());
							} catch (const EmptyQuery &e)
							{
								// Constructs the error
								std::string error = "domain [";
								error += address.getDomain();
								error += "] was not found on this server.";

								// Sends the error response
								client->sendResponse(
									false,
									POP3ResponseType::PRT_AUTH_FAIL,
									"",
									nullptr,
									reinterpret_cast<void *>(const_cast<char *>(error.c_str()))
								);
								continue;
							}
						} else
						{
							command.c_Args[0] += '@';
							command.c_Args[0] += _SMTP_DEF_DOMAIN;
							address.parse(command.c_Args[0]);
						}

						// Checks if the username with the domain is found
						// - in our redis database
						try {
							session.s_Account = AccountShortcut::findRedis(redis.get(), address.getDomain(), address.getUsername());
						} catch (const EmptyQuery &e)
						{
							// Constructs the error
							std::string error = "user [";
							error += address.e_Address;
							error += "] was not found on this server.";

							// Sends the error response
							client->sendResponse(
								false,
								POP3ResponseType::PRT_AUTH_FAIL,
								"",
								nullptr,
								reinterpret_cast<void *>(const_cast<char *>(error.c_str()))
							);
							continue;
						}

						// Stores the user in the session, and sends
						// - the continue response
						session.s_User = command.c_Args[0];
						client->sendResponse(true, POP3ResponseType::PRT_USER_DONE);
						continue;
					}
					case POP3CommandType::PCT_CAPA:
					{
						// Sends the list of capabilities
						client->sendResponse(
							true,
							POP3ResponseType::PRT_CAPA,
							"",
							&server.s_Capabilities,
							nullptr
						);
						continue;
					}
					case POP3CommandType::PCT_STAT:
					{
						
					}
					case POP3CommandType::PCT_UNKNOWN:
					{
						throw InvalidCommand("Command not found");
					}
				}
			} catch (const InvalidCommand& e)
			{
				client->sendResponse(false, POP3ResponseType::PRT_COMMAND_INVALID);
			}
		}
	pop3_session_end:
		return;
	}

	/**
	 * Stops the pop3 server
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void P3Server::shutdown(void)
	{
		Logger &logger = this->s_Logger;

		logger << "POP3 Server wordt afgesloten ..." << ENDL;
		int64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
		this->s_Socket.shutdown(&this->s_Running, &this->s_Run);
		int64_t end = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
		logger << "POP3 Server afgesloten in " << end - start << " milliseconden" << ENDL;
	}
}
