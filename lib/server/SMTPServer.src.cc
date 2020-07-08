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

namespace FSMTP::Server
{
	SMTPServer::SMTPServer(const int32_t &port, const bool& s_UseESMTP, const int32_t &s_Opts):
		s_Socket(SMTPSocketType::SST_SERVER, port), s_UseESMTP(s_UseESMTP), s_Logger("SMTPServer", LoggerLevel::INFO), s_Opts(s_Opts)
	{
		// Sets some default values and after that 
		// - we start the socket server, and create
		// - the listening thread
		this->s_IsRunning = false;
		this->s_ShouldBeRunning = true;

		std::bitset<32> optsCheck(s_Opts);
		this->s_Logger << DEBUG << "Ontvangen configuratie: 0b" << optsCheck << ENDL << CLASSIC;
		if (BINARY_COMPARE(this->s_Opts, _SERVER_OPT_ENABLE_AUTH))
		{
			this->s_Logger << INFO << "SMTP Authenticatie is toegestaan." << ENDL << CLASSIC;
			this->s_Services.push_back({
				"AUTH",
				{"LOGIN", "DIGEST-MD5", "PLAIN"}
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

	SMTPSocket &SMTPServer::getSocket(void)
	{
		return this->s_Socket;
	}

	void SMTPServer::onClientSync(struct sockaddr_in *sAddr, int32_t fd, void *u)
	{
		SSL_CTX *sslCtx = nullptr;
		SSL *ssl = nullptr;
		int32_t executed = 0x0;
		SMTPServer &server = *reinterpret_cast<SMTPServer *>(u);
		SMTPServerSession session;

		// Creates the logger with the clients address
		// - so we can get awesome debug messages
		std::string prefix = "client:";
		prefix += inet_ntoa(sAddr->sin_addr);
		Logger logger(prefix, LoggerLevel::DEBUG);

		// Creates the database connection
		std::unique_ptr<CassandraConnection> database;
		try {
			database = std::make_unique<CassandraConnection>(_CASSANDRA_DATABASE_CONTACT_POINTS);
			logger << "Verbinding met Apache Cassandra gemaakt." << ENDL;
		} catch (const std::runtime_error &e)
		{
			logger << FATAL << "Kon geen verbinding met Apache Cassandra maken, verbinding wordt gesloten !" << ENDL << CLASSIC;
			goto smtp_server_close_conn;
		}

		// Prints the initial client information and then
		// - we sent the initial hello message to the SMTP client
		logger << "onClientSync() aangeroepen, verbinding gemaakt." << ENDL;

		{ // ( Initial message )
			ServerResponse response(SRC_INIT, server.s_UseESMTP, nullptr);
			std::string message;
			response.build(message);
			SMTPSocket::sendString(fd, nullptr, false, message);
		}

		// Starts the infinite reading and writing loop
		// - in here we will handle commands and send responses
		while (true)
		{
			// Receives the data from the client
			std::string rawData;
			try
			{
				SMTPSocket::receiveString(fd, ssl, session.getSSLFlag(), false, rawData);
			} catch(const std::runtime_error &e)
			{
				logger << FATAL << "An exception occured: " << e.what() << ", closing connection !" << ENDL << CLASSIC;
				break;
			}

			// Performs the parsing of the command and then checks which
			// - method we need to call to perform the command
			ClientCommand command(rawData);
			DEBUG_ONLY(logger << DEBUG << "[pClass: " << command.c_CommandType << ", argC: " << command.c_Arguments.size() << "] -> C: " << rawData << ENDL << CLASSIC);

			// Builds the data struct which we cann pass to the actions
			BasicActionData actionData {
				command,
				sAddr,
				server.s_UseESMTP,
				fd,
				ssl
			};

			if (command.c_CommandType == ClientCommandType::CCT_QUIT)
			{
				std::string message;
				ServerResponse resp(SMTPResponseCommand::SRC_QUIT_RESP, server.s_UseESMTP, nullptr);
				resp.build(message);
				SMTPSocket::sendString(fd, ssl, session.getSSLFlag(), message);
				break;
			}

			try 
			{
				switch (command.c_CommandType)
				{
					case ClientCommandType::CCT_HELO:
					{
						actionHelloInitial(actionData);
						break;
					}
					case ClientCommandType::CCT_START_TLS:
					{
						DEBUG_ONLY(logger << DEBUG << "Veilige verbinding wordt aangevraagd." << ENDL << CLASSIC);

						// Sends the message that the client may
						// - proceed with the STARTTLS stuff
						std::string message;
						ServerResponse resp(SMTPResponseCommand::SRC_READY_START_TLS, server.s_UseESMTP, nullptr);
						resp.build(message);
						SMTPSocket::sendString(fd, ssl, session.getSSLFlag(), message);

						// Upgrades the socket connection to use TLS,
						// - after that we print the message to the console
						try {
							SMTPSocket::upgradeToSSL(fd, ssl, sslCtx);
							DEBUG_ONLY(logger << DEBUG << "Verbinding is succesvol beveiligd." << ENDL << CLASSIC);

							// Updates the flags
							session.setSSLFlag();
						} catch (const std::runtime_error &e)
						{
							DEBUG_ONLY(logger << ERROR << "Verbinding kan niet worden beveiligd, daarom wordt de verbinding gesloten." << ENDL << CLASSIC);
							goto smtp_server_close_conn;
						}
						break;
					}
					case ClientCommandType::CCT_MAIL_FROM:
					{
						actionMailFrom(actionData, logger, database, session);
						break;
					}
					case ClientCommandType::CCT_RCPT_TO:
					{
						actionRcptTo(actionData, logger, database, session);
						break;
					}
					case ClientCommandType::CCT_DATA:
					{
						// Sends the response that the client
						// - may start sending the data
						{
							std::string message;
							ServerResponse resp(SMTPResponseCommand::SRC_DATA_START, server.s_UseESMTP, nullptr);
							resp.build(message);
							SMTPSocket::sendString(fd, ssl, session.getSSLFlag(), message);
						}

						// Receives the full message
						std::string data;
						SMTPSocket::receiveString(fd, ssl, session.getSSLFlag(), true, data);

						MIME::joinMessageLines(data);
						MIME::parseRecursive(data, session.s_TransportMessage, 0);
						FullEmail::print(session.s_TransportMessage, logger);

						// Sends the data finished command
						// - to indicate the body was received
						std::string message;
						ServerResponse resp(SMTPResponseCommand::SRC_DATA_END, server.s_UseESMTP, nullptr);
						resp.build(message);
						SMTPSocket::sendString(fd, ssl, session.getSSLFlag(), message);
						break;
					}
					case ClientCommandType::CCT_UNKNOWN:
					{
						std::string message;
						ServerResponse resp(SMTPResponseCommand::SRC_SYNTAX_ERR_INVALID_COMMAND, server.s_UseESMTP, nullptr);
						resp.build(message);
						SMTPSocket::sendString(fd, ssl, session.getSSLFlag(), message);
						break;
					}
				}
			} catch (const SyntaxException &e)
			{
				std::string message;
				ServerResponse resp(SMTPResponseCommand::SRC_SYNTAX_ARG_ERR, e.what());
				resp.build(message);
				SMTPSocket::sendString(fd, ssl, session.getSSLFlag(), message);
			} catch (const FatalException &e)
			{
				logger << FATAL << "Fatal exception: " << e.what() << ENDL << CLASSIC;
				goto smtp_server_close_conn;
			} catch (const std::runtime_error &e)
			{
				logger << FATAL << "Fatal exception ( runtime ): " << e.what() << ENDL << CLASSIC;
				goto smtp_server_close_conn;
			}
		}

	smtp_server_close_conn:

		// Closes the connection with the client, and if
		// - an message needs to be sent, please do this before
		shutdown(fd, SHUT_RDWR);
		logger << WARN << "Verbinding is gesloten." << ENDL << CLASSIC;
		free(sAddr);

		if (session.getSSLFlag())
		{
			SSL_free(ssl);
			SSL_CTX_free(sslCtx);
		}
	}

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
