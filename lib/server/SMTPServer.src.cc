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

#include "SMTPServer.src.h"

extern bool _forceLoggerNCurses;

static std::atomic<int> _serverThreadCount = 0;
static std::atomic<int> _emailsHandled = 0;

namespace FSMTP::Server
{
	/**
	 * Default constructor for the SMTPServer
	 *
	 * @Param {const int32_t} port
	 * @Param {const bool} s_UseESMTP
	 * @Param {const int32_t} s_Opts
	 * @Param {const int32_t} s_RedisPort
	 * @Param {ocnst std::string &} s_RedisHost
	 */
	SMTPServer::SMTPServer(
		const int32_t port,
		const bool s_UseESMTP,
		const int32_t s_Opts,
		const int32_t s_RedisPort,
		const std::string &s_RedisHost
	):
		s_Socket(port),
		s_UseESMTP(s_UseESMTP),
		s_Logger("SMTPServer", LoggerLevel::INFO),
		s_Opts(s_Opts),
		s_RedisPort(s_RedisPort),
		s_RedisHost(s_RedisHost)
	{
		// Sets some default values and after that 
		// - we start the socket server, and create
		// - the listening thread
		this->s_IsRunning = false;
		this->s_ShouldBeRunning = true;

		std::bitset<32> optsCheck(s_Opts);
		this->s_Logger << "Ontvangen configuratie: 0b" << optsCheck << ENDL;
		if (BINARY_COMPARE(this->s_Opts, _SERVER_OPT_ENABLE_AUTH))
		{
			this->s_Logger << INFO << "SMTP Authenticatie is toegestaan." << ENDL << CLASSIC;
			this->s_Services.push_back({
				"AUTH",
				{"PLAIN"}
			});
		}
		if (BINARY_COMPARE(this->s_Opts, _SERVER_OPT_ENABLE_PIPELINING))
		{
			this->s_Logger << INFO << "Pipelining is toegestaan." << ENDL << CLASSIC;
			this->s_Services.push_back({
				"PIPELINING",
				{}
			});
		}
		if (BINARY_COMPARE(this->s_Opts, _SERVER_OPT_ENABLE_TLS))
		{
			this->s_Logger << INFO << "TLS Verbinding is toegestaan." << ENDL << CLASSIC;
			this->s_Services.push_back({
				"STARTTLS",
				{}
			});
		}

		this->s_Socket.startListening();
		this->s_Socket.startAcceptorSync(
			&SMTPServer::onClientSync, 
			30, 
			true, 
			this->s_ShouldBeRunning,
			this->s_IsRunning,
			reinterpret_cast<void *>(this)
		);

		this->s_Logger << "Fannst ESMTP Server luistert nu op port " << port << '.' << ENDL;
	}

	/**
	 * Gets the socket
	 *
	 * @Param {void}
	 * @Return {SMTPSocket &} socket
	 */
	SMTPSocket &SMTPServer::getSocket(void)
	{
		return this->s_Socket;
	}

	/**
	 * The method which gets called when an client
	 * - has connected
	 *
	 * @Param {struct sockaddr_in *} sockaddr
	 * @Param {int32_t} fd
	 * @Param {void *} u
	 * @Return {void}
	 */
	void SMTPServer::onClientSync(struct sockaddr_in *sAddr, int32_t fd, void *u)
	{
		_serverThreadCount++;
		if (_forceLoggerNCurses) NCursesDisplay::setThreads(_serverThreadCount);

		SMTPServer &server = *reinterpret_cast<SMTPServer *>(u);
		SMTPServerSession session;

		// Creates the logger with the clients address
		// - so we can get awesome debug messages
		std::string prefix = "SMTPClient:";
		prefix += inet_ntoa(sAddr->sin_addr);
		Logger logger(prefix, LoggerLevel::INFO);
		SMTPServerClientSocket client(fd, *sAddr, logger);

		// =================================
		// Connects to redis
		//
		// Creates the connection to the
		// - redis server, which we will
		// - later use to get users and
		// - the domains
		// =================================
		
		std::unique_ptr<RedisConnection> redis;
		try
		{
			redis = std::make_unique<RedisConnection>(
				server.s_RedisHost.c_str(),
				server.s_RedisPort
			);
		} catch (const std::runtime_error &e)
		{
			logger << FATAL << "Kon geen verbinding met Redis maken, verbinding wordt gesloten !" << ENDL << CLASSIC;
			goto smtp_server_close_conn;
		}

		// Sends the initial message
		client.sendResponse(SMTPResponseType::SRC_GREETING);

		// Starts the infinite reading and writing loop
		// - in here we will handle commands and send responses
		while (true)
		{
			try 
			{
				// Reads the command, parses it
				// - and quits when an error is thrown
				ClientCommand command = ClientCommand(client.readUntillNewline(false));

				// Checks how we should respond to the command
				switch (command.c_CommandType)
				{
					case ClientCommandType::CCT_QUIT:
					{
						// Writes the goodbye message, and closes the connection
						client.sendResponse(SMTPResponseType::SRC_QUIT_GOODBYE);
						goto smtp_server_close_conn;
					}
					case ClientCommandType::CCT_HELO:
					{ // ( Simple hello command )
						// Checks we should handle the hello command
						// - this is always possible, except when the 
						// - hello command was already transmitted
						if (!session.getAction(_SMTP_SERV_PA_HELO))
						{
							// Checks if there are arguments, else throw syntax
							// - error because the server domain is required
							if (command.c_Arguments.size() < 1)
								throw SyntaxException("Empty HELO command not allowed");

							// Sets the action flag so we can not allow
							// - hello again
							session.setAction(_SMTP_SERV_PA_HELO);

							// Writes the response to the client
							client.sendResponse(
								SMTPResponseType::SRC_HELO,
								"",
								reinterpret_cast<void *>(sAddr),
								nullptr
							);

							// Continues to the next round
							continue;
						}
						break;
					}
					case ClientCommandType::CCT_EHLO:
					{ // ( Extended Hello command )
						// Checks if we should handle the hello command
						// - since it is only allowed when we've not sent it
						// - or starttls is called
						if (!session.getAction(_SMTP_SERV_PA_HELO))
						{
							// Checks if there are arguments, else throw syntax
							// - error because the server domain is required
							if (command.c_Arguments.size() < 1)
								throw SyntaxException("Empty HELO command not allowed");

							// Sets the action flag so we can not allow
							// - hello again
							session.setAction(_SMTP_SERV_PA_HELO);

							// Writes the response to the client
							client.sendResponse(
								SMTPResponseType::SRC_EHLO,
								"",
								reinterpret_cast<void *>(sAddr),
								&server.s_Services
							);

							// Continues to the next round
							continue;
						}
						break;
					}
					case ClientCommandType::CCT_START_TLS:
					{ // ( StartTLS command )

						// Checks if we're allowed to do this, this needs to happen
						// - after the hello message
						if (!session.getAction(_SMTP_SERV_PA_HELO))
							throw CommandOrderException("EHLO/HELO first.");

						// Sends the proceed command to the client
						// - and upgrades the socket
						client.sendResponse(SMTPResponseType::SRC_START_TLS);
						client.upgrade();

						// Clears the hello action since hello is now allowed
						// - to be sent again
						session.clearAction(_SMTP_SERV_PA_HELO);
						break;
					}
					case ClientCommandType::CCT_MAIL_FROM:
					{
						// Checks if the mail from command is allowed, this is
						// - only possible if the initial greeting was performed
						if (!session.getAction(_SMTP_SERV_PA_MAIL_FROM))
						{
							// Checks if we're allowed to perform the mail from
							// - command, if not throw exception
							if (!session.getAction(_SMTP_SERV_PA_HELO))
								throw CommandOrderException("EHLO/HELO first.");

							// Parses the mail address, if this fails
							// - throw syntax exception
							try {
								session.s_TransportMessage.e_TransportFrom.parse(
									command.c_Arguments[0]);
							}
							catch (const std::runtime_error &e)
							{
								std::string message = "Invalid email address: ";
								message += e.what();
								message += '.';
								throw SyntaxException(message);
							}

							// Parses the domain and finds if the domain
							// - is local on our server
							std::string domain = session.s_TransportMessage.e_TransportFrom.getDomain();
							try {
								// Gets the domain from redis and checks if the auth flag is set
								// - if not we throw command order exception, since relaying
								// - requires authentication
								LocalDomain localDomain = LocalDomain::findRedis(domain, redis.get());
								if (!session.getFlag(_SMTP_SERV_SESSION_AUTH_FLAG))
									throw CommandOrderException("AUTH first.");

								// Sets the relaying flag
								session.setFlag(_SMTP_SERV_SESSION_RELAY_FLAG);
							}
							catch (const EmptyQuery &e)
							{
								// No worry's is allowed
							}

							// Sends the response and sets the 
							// - action flag
							client.sendResponse(
								SMTPResponseType::SRC_MAIL_FROM,
								"",
								reinterpret_cast<void *>(
									const_cast<char *>(
										session.s_TransportMessage.e_TransportFrom.e_Address.c_str()
									)
								),
								nullptr
							);
							session.setAction(_SMTP_SERV_PA_MAIL_FROM);
						}
						break;
					}
					case ClientCommandType::CCT_RCPT_TO:
					{
						// Checks if we're allowed to perform the rcpt to command,
						// - this is only allowed if mail from is sent, and hello performed
						if (!session.getAction(_SMTP_SERV_PA_RCPT_TO))
						{
							// Checks if we're allowed to perform it, else
							// - throws order error
							if (!session.getAction(_SMTP_SERV_PA_HELO))
								throw CommandOrderException("EHLO/HELLO first.");
							if (!session.getAction(_SMTP_SERV_PA_MAIL_FROM))
								throw CommandOrderException("MAIL FROM first.");


							// Parses the mail address, if this fails
							// - throw syntax exception
							try {
								session.s_TransportMessage.e_TransportTo.parse(
									command.c_Arguments[0]);
							}
							catch (const std::runtime_error &e)
							{
								std::string message = "Invalid email address: ";
								message += e.what();
								message += '.';
								throw SyntaxException(message);
							}

							// Checks if we're relaying, and if not if the user
							// - is inside of the database
							if (!session.getFlag(_SMTP_SERV_SESSION_RELAY_FLAG))
							{
								try {
									// Finds the user shortcut
									session.s_ReceivingAccount = AccountShortcut::findRedis(
										redis.get(), 
										session.s_TransportMessage.e_TransportTo.getDomain(),
										session.s_TransportMessage.e_TransportTo.getUsername()
									);
								} catch (const EmptyQuery &e)
								{
									// Sends the not found error to the client
									client.sendResponse(
										SMTPResponseType::SRC_REC_NOT_LOCAL,
										"",
										reinterpret_cast<void *>(
											const_cast<char *>(
												session.s_TransportMessage.e_TransportTo.e_Address.c_str()
											)
										),
										nullptr
									);
									goto smtp_server_close_conn;
								}
							}

							// Sends the response and sets the 
							// - action flag
							client.sendResponse(
								SMTPResponseType::SRC_RCPT_TO,
								"",
								reinterpret_cast<void *>(
									const_cast<char *>(
										session.s_TransportMessage.e_TransportTo.e_Address.c_str()
									)
								),
								nullptr
							);
							session.setAction(_SMTP_SERV_PA_RCPT_TO);
						}
						break;
					}
					case ClientCommandType::CCT_AUTH:
					{
						// Checks if we're allowed to do this, actually only possible
						// - only possible after helo
						if (!session.getAction(_SMTP_SERV_PA_AUTH_PERF))
						{
							// Checks if we're allowed to perform this command
							// - based on the command order
							if (!session.getAction(_SMTP_SERV_PA_HELO))
								throw CommandOrderException("EHLO/HELLO first.");

							// Checks if the argument count is valid
							if (command.c_Arguments.size() < 2)
								throw SyntaxException("Specify type, and hash.");

							// =======================================
							// Validates the entry
							//
							// Checks if the password etcetera is
							// - correct, if not throw err
							// =======================================

							// Gets the username and password from the auth
							// - hash
							std::string username, password;
							std::tie(username, password ) = getUserAndPassB64(
								command.c_Arguments[1]
							);

							// Gets the users password

							// Sets the flag and sends the success command
							session.setAction(_SMTP_SERV_PA_AUTH_PERF);
							session.setFlag(_SMTP_SERV_SESSION_AUTH_FLAG);
							client.sendResponse(SRC_AUTH_SUCCESS);
							continue;
						}
						break;
					}
					case ClientCommandType::CCT_DATA:
					{
						// Checks if we're allowed to perform this command
						if (!session.getAction(_SMTP_SERV_PA_DATA_START))
						{
							// Checks if we're allowed to perform this command
							// - based on the command order
							if (!session.getAction(_SMTP_SERV_PA_HELO))
								throw CommandOrderException("EHLO/HELLO first.");
							if (!session.getAction(_SMTP_SERV_PA_MAIL_FROM))
								throw CommandOrderException("MAIL FROM first.");
							if (!session.getAction(_SMTP_SERV_PA_RCPT_TO))
								throw CommandOrderException("RCPT TO first.");

							// Sets the data start flag
							session.setAction(_SMTP_SERV_PA_DATA_START);

							// Sends the data start command, 
							// - and starts receiving the body
							client.sendResponse(SMTPResponseType::SRC_DATA_START);
							std::string rawTransportmessage = client.readUntillNewline(true);

							// Joins the message lines and starts the recursive parser
							MIME::joinMessageLines(rawTransportmessage);
							MIME::parseRecursive(rawTransportmessage, session.s_TransportMessage, 0);

							// Checks if we need to add it to the relay queue, or
							// - the normal storage queue

							if (!session.getFlag(_SMTP_SERV_SESSION_RELAY_FLAG))
							{
								// Adds the user information to the transport message
								session.s_TransportMessage.e_OwnersUUID = session.s_ReceivingAccount.a_UUID;
								session.s_TransportMessage.generateMessageUUID();
								session.s_TransportMessage.e_Bucket = FullEmail::getBucket();
								session.s_TransportMessage.e_Type = EmailType::ET_INCOMMING;

								// Pushes the email to the storage queue
								_emailStorageMutex.lock();
								_emailStorageQueue.push_back(session.s_TransportMessage);
								_emailStorageMutex.unlock();
							}

							// Sends the response and sets the 
							// - action flag
							client.sendResponse(
								SMTPResponseType::SRC_DATA_END,
								"",
								reinterpret_cast<void *>(
									const_cast<char *>(
										session.s_TransportMessage.e_MessageID.c_str()
									)
								),
								nullptr
							);
							session.setAction(_SMTP_SERV_PA_DATA_END);

							// Prints that the email is received to the console
							logger << "Email received: " << session.s_TransportMessage.e_MessageID << ENDL;
						}
						break;
					}
					case ClientCommandType::CCT_UNKNOWN:
					default:
					{
						client.sendResponse(SMTPResponseType::SRC_INVALID_COMMAND);
						break;
					}
				}
			} catch (const CommandOrderException &e)
			{
				logger << FATAL << "Command order error: " << e.what() << ENDL << CLASSIC;
				client.sendResponse(
					SMTPResponseType::SRC_ORDER_ERR,
					e.what(),
					nullptr,
					nullptr
				);
			} catch (const SMTPTransmissionError &e)
			{
				logger << FATAL << "Transmission error: " << e.what() << ENDL << CLASSIC;
				break;
			} catch (const SyntaxException &e)
			{
				// Prints the error to the console and sends the syntax error
				logger << ERROR << "Syntax error: " << e.what() << ENDL << CLASSIC;
				client.sendResponse(
					SMTPResponseType::SRC_SYNTAX_ERR,
					e.what(),
					nullptr,
					nullptr
				);
			} catch (const FatalException &e)
			{
				logger << FATAL << "Fatal exception: " << e.what() << ENDL << CLASSIC;
				break;
			} catch (const std::runtime_error &e)
			{
				logger << FATAL << "Fatal exception ( runtime ): " << e.what() << ENDL << CLASSIC;
				break;
			}
		}

	smtp_server_close_conn:
		delete sAddr;

		// Updates the thread count, and emails handled
		_serverThreadCount--;
		if (_forceLoggerNCurses)
			NCursesDisplay::setThreads(_serverThreadCount);
		if (_forceLoggerNCurses)
		{
			_emailsHandled++;
			NCursesDisplay::setEmailsHandled(_emailsHandled);
		}
	}

	/**
	 * Closes the SMTP Server
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void SMTPServer::shutdownServer(void)
	{
		// Stores the start time and closes the threads
		// - after that we will see the duration of the
		// - closing process since some threads may be open
		std::size_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		this->s_Logger << WARN << "Threads worden gesloten, een moment geduld a.u.b." << ENDL << INFO;
		this->s_Socket.closeThread(
			this->s_ShouldBeRunning,
			this->s_IsRunning
		);
		std::size_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		this->s_Logger << WARN << "Threads gesloten in " << now - start << " milliseconden." << ENDL << INFO;
	}
}
