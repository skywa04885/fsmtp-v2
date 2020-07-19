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
		s_Logger("P3Serv", LoggerLevel::INFO)
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
		this->s_Logger << "Server wordt gestart ..." << ENDL;
		this->s_Socket.startListening(
			&this->s_Running,
			&this->s_Run,
			&P3Server::acceptorCallback,
			this
		);
		this->s_Logger << "Server is gestart op port " << (secure ? 995 : 110) << ENDL;
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
							throw SyntaxError("PASS requires one argument");
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
								nullptr, nullptr,
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
								nullptr, nullptr,
								reinterpret_cast<void *>(const_cast<char *>("invalid password"))
							);
							continue;
						}

						// Gets the emails available
						session.s_References = EmailShortcut::gatherAllReferencesWithSize(
							cassandra.get(),
							0,
							120,
							session.s_Account.a_Domain,
							session.s_Account.a_UUID
						);

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
							throw SyntaxError("USER requires one argument");
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
									nullptr, nullptr,
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
								nullptr, nullptr,
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
							nullptr, nullptr
						);
						continue;
					}
					case POP3CommandType::PCT_STAT:
					{
						// Gets the data and builds the response
						int64_t octets;
						for (const std::tuple<CassUuid, int64_t, int64_t> &pair : session.s_References)
							octets += std::get<1>(pair);

						std::string response = std::to_string(session.s_References.size());
						response += ' ';
						response += std::to_string(octets);

						// Sends the stat response
						client->sendResponse(
							true,
							POP3ResponseType::PRT_STAT,
							response, nullptr, nullptr, nullptr
						);
						continue;
					}
					case POP3CommandType::PCT_UIDL:
					{
						std::vector<POP3ListElement> list = {};

						std::size_t i = 0;
						for (const std::tuple<CassUuid, int64_t, int64_t> &ref : session.s_References)
							list.push_back(POP3ListElement{
								i++,
								std::to_string(cass_uuid_timestamp(std::get<0>(ref)))
							});

						// Sends the response
						client->sendResponse(
							true,
							POP3ResponseType::PRT_UIDL,
							"", nullptr, &list, nullptr
						);

						break;
					}
					case POP3CommandType::PCT_LIST:
					{
						std::vector<POP3ListElement> list = {};

						std::size_t i = 0;
						for (const std::tuple<CassUuid, int64_t, int64_t> &ref : session.s_References)
							list.push_back(POP3ListElement{
								i++,
								std::to_string(std::get<1>(ref))
							});

						// Sends the response
						client->sendResponse(
							true,
							POP3ResponseType::PRT_LIST,
							"", nullptr, &list, nullptr
						);

						break;
					}
					case POP3CommandType::PCT_RETR:
					{
						// Checks if there is an argument
						if (command.c_Args.size() < 1)
							throw SyntaxError("RETR requires one argument");

						// Tries to parse the argument, and checks if the index is too
						// - large by comparing it to the vector
						std::size_t i;
						try {
							i = std::stol(command.c_Args[0]);
						} catch (const std::invalid_argument &e)
						{
							throw SyntaxError("Invalid index for RETR");
						}
						if (i > session.s_References.size())
						{
							std::string error = "Max index ";
							error += std::to_string(session.s_References.size());
							throw SyntaxError(error);
						}

						// Gets the UUID from the specified message, and then
						// - query's the raw message
						const CassUuid &uuid = std::get<0>(session.s_References[i]);
						RawEmail raw = RawEmail::get(
							cassandra.get(), 
							session.s_Account.a_Domain,
							session.s_Account.a_UUID,
							uuid,
							std::get<2>(session.s_References[i])
						);

						// Prepares the email contents
						if (raw.e_Content.substr(raw.e_Content.size() - 2) != "\r\n")
							raw.e_Content += "\r\n";
						raw.e_Content += ".\r\n";

						// Sends the email contents
						client->sendResponse(
							true,
							POP3ResponseType::PRT_RETR,
							"",
							nullptr, nullptr,
							reinterpret_cast<void *>(&std::get<1>(session.s_References[i]))
						);
						client->sendString(raw.e_Content);

						continue;
					}
					case POP3CommandType::PCT_UNKNOWN:
					{
						throw InvalidCommand("Command not found");
					}
				}
			} catch (const InvalidCommand& e)
			{
				client->sendResponse(false, POP3ResponseType::PRT_COMMAND_INVALID);
			} catch (const SyntaxError& e)
			{
				client->sendResponse(
					false,
					POP3ResponseType::PRT_SYNTAX_ERROR,
					"",
					nullptr, nullptr,
					reinterpret_cast<void *>(const_cast<char *>(e.what()))
				);
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