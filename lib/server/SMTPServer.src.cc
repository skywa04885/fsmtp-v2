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

		SMTPServerClientSocket client(fd, *sAddr);
		SMTPServer &server = *reinterpret_cast<SMTPServer *>(u);
		SMTPServerSession session;

		// Creates the logger with the clients address
		// - so we can get awesome debug messages
		std::string prefix = "client:";
		prefix += inet_ntoa(sAddr->sin_addr);
		Logger logger(prefix, LoggerLevel::DEBUG);

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

		// Starts the infinite reading and writing loop
		// - in here we will handle commands and send responses
		while (true)
		{
			try 
			{
				// Reads the command, parses it
				// - and quits when an error is thrown
				ClientCommand command = ClientCommand(client.readUntillNewline(true));

				// Checks how we should respond to the command
				switch (command.c_CommandType)
				{
					case ClientCommandType::CCT_HELO:
					{ // ( Simple hello command )
						// Checks we should handle the hello command
						// - this is always possible, except when the 
						// - hello command was already transmitted
						if (!session.getAction(_SMTP_SERV_PA_HELO))
						{
							session.setAction(_SMTP_SERV_PA_HELO);

							// Checks if there are arguments, else throw syntax
							// - error because the server domain is required
							if (command.c_Arguments.size() < 1)
								throw SyntaxException("Empty HELO command not allowed !");

							// Continues to the next round
							continue;
						}
					}
					case ClientCommandType::CCT_START_TLS:
					{ // ( Advenced hello command )
						break;
					}
					case ClientCommandType::CCT_MAIL_FROM:
					{
						break;
					}
					case ClientCommandType::CCT_RCPT_TO:
					{
						break;
					}
					case ClientCommandType::CCT_DATA:
					{
						break;
					}
					case ClientCommandType::CCT_UNKNOWN:
					{
						break;
					}
				}
			} catch (const SMTPTransmissionError &e)
			{
				logger << FATAL << "Transmission error: " << e.what() << ENDL << CLASSIC;
				break;
			} catch (const SyntaxException &e)
			{

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

		// Closes the connection with the client, and if
		// - an message needs to be sent, please do this before
		logger << WARN << "Verbinding is gesloten." << ENDL << CLASSIC;
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
