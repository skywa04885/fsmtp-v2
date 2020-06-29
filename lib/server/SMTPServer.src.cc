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
		this->s_Logger << DEBUG << "Options received: 0b" << optsCheck << ENDL << CLASSIC;
		if (BINARY_COMPARE(this->s_Opts, _SERVER_OPT_ENABLE_AUTH))
		{
			this->s_Logger << INFO << "Using SMTP Authentication" << ENDL << CLASSIC;
			this->s_Services.push_back({
				"AUTH",
				{"LOGIN", "DIGEST-MD5", "PLAIN"}
			});
		}
		if (BINARY_COMPARE(this->s_Opts, _SERVER_OPT_ENABLE_PIPELINING))
		{
			this->s_Logger << INFO << "Using SMTP pipelining" << ENDL << CLASSIC;
			this->s_Services.push_back({
				"PIPELINING",
				{}
			});
		}
		if (BINARY_COMPARE(this->s_Opts, _SERVER_OPT_ENABLE_TLS))
		{
			this->s_Logger << INFO << "Using SMTP STARTTLS" << ENDL << CLASSIC;
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

		this->s_Logger << "FSMTP listening on port: " << port << ENDL;
	}

	SMTPSocket &SMTPServer::getSocket(void)
	{
		return this->s_Socket;
	}

	void SMTPServer::onClientSync(std::shared_ptr<struct sockaddr_in> sockaddr, int32_t fd, void *u)
	{
		SMTPServer &server = *reinterpret_cast<SMTPServer *>(u);

		// Creates the logger with the clients address
		// - so we can get awesome debug messages
		std::string prefix = "client:";
		prefix += inet_ntoa(sockaddr.get()->sin_addr);
		Logger logger(prefix, LoggerLevel::DEBUG);

		// Prints the initial client information and then
		// - we sent the initial hello message to the SMTP client
		logger << "onClientSync() called, connection initialized !" << ENDL;

		{
			ServerResponse response(SRC_INIT, server.s_UseESMTP, nullptr);
			std::string message;
			response.build(message);
			SMTPSocket::sendString(fd, false, message);
		}
	}

	void SMTPServer::shutdown(void)
	{
		// Stores the start time and closes the threads
		// - after that we will see the duration of the
		// - closing process since some threads may be open
		std::size_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		this->s_Logger << WARN << "Closing threads ..." << ENDL << INFO;
		this->s_Socket.closeThread(
			this->s_ShouldBeRunning,
			this->s_IsRunning
		);
		std::size_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		this->s_Logger << WARN << "Threads closed successfully in " << now - start << "ms !" << ENDL << INFO;
	}
}