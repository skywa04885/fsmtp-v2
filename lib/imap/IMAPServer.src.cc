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

#include "IMAPServer.src.h"

namespace FSMTP::IMAP
{
	IMAPServer::IMAPServer(const int32_t plainPort, const int32_t securePort):
		s_Socket(plainPort, securePort), s_SecureRunning(false),
		s_PlainRunning(false), s_Run(true)
	{
		// Starts listening
		this->s_Socket.startListening(
			&this->s_PlainRunning,
			&this->s_SecureRunning,
			&this->s_Run,
			&IMAPServer::acceptorCallback,
			this
		);

		// Sets the shared capability's
		this->s_SecureCapabilities.push_back(IMAPCapability{
			"IMAP4rev1", nullptr
		});
		this->s_PlainCapabilities.push_back(IMAPCapability{
			"IMAP4rev1", nullptr
		});

		// Sets the secure only capability's
		this->s_SecureCapabilities.push_back(IMAPCapability{
			"LOGIN", nullptr
		});

		// Sets the plain only capability's
		this->s_PlainCapabilities.push_back(IMAPCapability{
			"STARTTLS", nullptr
		});
		this->s_PlainCapabilities.push_back(IMAPCapability{
			"LOGINDISABLED", nullptr
		});
	}

	void IMAPServer::acceptorCallback(
		std::unique_ptr<IMAPClientSocket> client,
		void *u
	)
	{
		IMAPServer &server = *reinterpret_cast<IMAPServer *>(u);
		std::string prefix = "IMAPAcceptor:";
		prefix += inet_ntoa(client->s_Addr.sin_addr);
		Logger logger(prefix, LoggerLevel::INFO);

		IMAPServerSession session;

		// ========================================
		// Connects to the databases
		//
		// Connects to Redis and Cassandra so we
		// - can receive users, messages etc
		// ========================================

		std::unique_ptr<CassandraConnection> cassandra;
		std::unique_ptr<RedisConnection> redis;

		try
		{
			cassandra = std::make_unique<CassandraConnection>(
				_CASSANDRA_DATABASE_CONTACT_POINTS);
		} catch (const std::runtime_error &e)
		{
			logger << FATAL << "Could not connect to Cassandra: " << e.what() << ENDL << CLASSIC;
			goto _imap_server_acceptor_end;
		}

		try
		{
			redis = std::make_unique<RedisConnection>(
				_REDIS_CONTACT_POINTS, _REDIS_PORT
			);
		} catch (const std::runtime_error &e)
		{
			logger << FATAL << "Could not connect to Redis: " << e.what() << ENDL << CLASSIC;
			goto _imap_server_acceptor_end;
		}

		// ========================================
		// Sends the initial greeting
		//
		// Greets the client, to inform that the
		// - server is ready
		// ========================================

		client->sendString(IMAPResponse::buildGreeting());

		// Starts the communication loop
		for (;;)
		{
			IMAPCommand command;
			try
			{
				std::string raw = client->readUntilCRLF();
				command.parse(raw);
			} catch (const SocketReadException &e)
			{
				logger << FATAL << "Could not read data" << ENDL << CLASSIC;
				break;
			} catch (const std::length_error &e)
			{
				logger << FATAL << "Length error (most likely CMD sucker): " << e.what() << ENDL << CLASSIC;
				break;
			} catch (const IMAPBad &e)
			{
				client->sendString(IMAPResponse::buildBad(command.c_Index, e.what()));
				continue;
			}

			// ========================================
			// Handles the commands
			//
			// Starts the communication
			// ========================================

			try
			{
				switch (command.c_Type)
				{
					// ===============================================
					// Handles the 'UID' command
					//
					// Shows the folders etcetera, returns: 
					// - OK: select completed, now in selected state
					// - NO: select failure, not in authenticated sta
					// te, can't access mailbox
					// - BAD: Command unknown or arguments invalid
					// ===============================================
					case IMAPCommandType::ICT_SELECT:
					{
						MAILBOX_HANDLER::select(
							client.get(),
							command,
							session,
							redis.get(),
							cassandra.get()
						);
						continue;
					}
					// ===============================================
					// Handles the 'SELECT' command
					//
					// Shows the folders etcetera, returns: 
					// - OK: select completed, now in selected state
					// - NO: select failure, not in authenticated sta
					// te, can't access mailbox
					// - BAD: Command unknown or arguments invalid
					// ===============================================
					case IMAPCommandType::ICT_SELECT:
					{
						MAILBOX_HANDLER::select(
							client.get(),
							command,
							session,
							redis.get(),
							cassandra.get()
						);
						continue;
					}
					// ===============================================
					// Handles the 'LSUB' command
					//
					// Shows the folders etcetera, returns: 
					// - OK: lsub completed
					// - NO: list failure: can't list that reference
					// or name
					// - BAD: Command unknown or arguments invalid
					// ===============================================
					case IMAPCommandType::ICT_LSUB:
					{
						MAILBOX_HANDLER::lsub(
							client.get(),
							command,
							session,
							redis.get(),
							cassandra.get()
						);
						continue;
					}
					// ===============================================
					// Handles the 'LIST' command
					//
					// Shows the folders etcetera, returns: 
					// - OK: List completed
					// - NO: list failure: can't list that reference
					// or name
					// - BAD: Command unknown or arguments invalid
					// ===============================================
					case IMAPCommandType::ICT_LIST:
					{
						MAILBOX_HANDLER::list(
							client.get(),
							command,
							session,
							redis.get(),
							cassandra.get()
						);
						continue;
					}
					// ===============================================
					// Handles the 'STATTTLS' command
					//
					// Shows what the server is capable of
					// ===============================================
					case IMAPCommandType::ICT_STARTTLS:
					{
						// Sends the message, and upgrades
						client->sendString(IMAPResponse::buildCompleted(
							command.c_Index, IMAPCommandType::ICT_STARTTLS));
						client->upgrade();
						continue;
					}
					// ===============================================
					// Handles the 'CAPABILITY' command
					//
					// Shows what the server is capable of
					// ===============================================
					case IMAPCommandType::ICT_CAPABILITY:
					{
						// Checks if we are in plain or secure mode
						// - this effects the kind of commands we may use
						if (client->s_UseSSL)
						{
							client->sendString(IMAPResponse::buildCapabilities(
								command.c_Index, server.s_SecureCapabilities));
						} else
						{
							client->sendString(IMAPResponse::buildCapabilities(
								command.c_Index, server.s_PlainCapabilities));
						}
						continue;
					}
					// ===============================================
					// Handles the 'LOGIN' command
					//
					// Authenticates the user, responses:
					// - OK: Login completed, now in authenticated
					// state
					// - NO: Login failure: user or password rejected
					// - BAD: Command unknown / arguments invalid
					// ===============================================
					case IMAPCommandType::ICT_LOGIN:
					{
						AUTH_HANDLER::login(
							client.get(),
							command,
							session,
							redis.get(),
							cassandra.get()
						);
						continue;
					}
					// ===============================================
					// Handles the 'LOGOUT' command
					//
					// Closes the session, and shuts the connection
					// - down
					// ===============================================
					case IMAPCommandType::ICT_LOGOUT:
					{
						// Sends the message and terminates
						goto _imap_server_acceptor_end;
					}
					// ===============================================
					// Handles the 'NOOP' command
					//
					// Resets the timer
					// ===============================================
					case IMAPCommandType::ICT_NOOP:
					{
						client->sendString(IMAPResponse::buildCompleted(command.c_Index, 
							IMAPCommandType::ICT_NOOP));
						continue;
					}
					// ===============================================
					// Handles the unknown commands
					//
					// Throws unknown command error
					// ===============================================
					case IMAPCommandType::ICT_UNKNOWN:
					default:
					{
						throw IMAPBad("Command not implemented, fuck !");
					}
				}
			} catch (const IMAPBad& e)
			{
				client->sendString(IMAPResponse::buildBad(command.c_Index, e.what()));
			} catch (const IMAPNo& e)
			{
				client->sendString(IMAPResponse::buildNo(command.c_Index, e.what()));
			} catch (const DatabaseException &e)
			{
				logger << FATAL << "Database exception: " << e.what() << ENDL << CLASSIC;
				goto _imap_server_acceptor_end;
			}
		}

	_imap_server_acceptor_end: return;
	}
}