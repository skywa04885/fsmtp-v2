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

using namespace FSMTP::Server;

SMTPServer::SMTPServer() noexcept: s_Logger("FSMTP-V2/ESMTP", LoggerLevel::INFO) {
	const char *maxSize = "10000000";
	
	this->s_PlainServices.push_back({"AUTH", {"PLAIN"}});
	this->s_PlainServices.push_back({"STARTTLS", {}});
	this->s_PlainServices.push_back({"SMTPUTF8", {}});
	this->s_PlainServices.push_back({"SIZE", {maxSize}});
	this->s_PlainServices.push_back({"ENHANCEDSTATUSCODES", {}});

	this->s_SecureServices.push_back({"AUTH", {"PLAIN"}});
	this->s_SecureServices.push_back({"SMTPUTF8", {}});
	this->s_SecureServices.push_back({"SIZE", {maxSize}});
	this->s_SecureServices.push_back({"ENHANCEDSTATUSCODES", {}});
}

SMTPServer::~SMTPServer() noexcept {}

SMTPServer &SMTPServer::createContext() {
	auto &sslCtx = this->s_SSLContext;

	sslCtx = Global::getSSLContext(SSLv23_server_method());

	return *this;
}

SMTPServer &SMTPServer::connectDatabases() {
	auto &logger = this->s_Logger;
	auto &cass = this->s_Cassandra;

	try {
		cass = Global::getCassandra();
		logger << _BASH_SUCCESS_MARK << "Connected to cassandra" << ENDL;
	} catch (const runtime_error &err) {
		throw runtime_error(EXCEPT_DEBUG(CassandraConnection::getError(cass->c_ConnectFuture)));
	}

	try {
		this->s_Redis = Global::getRedis();
		logger << _BASH_SUCCESS_MARK << "Connected to redis" << ENDL;
	} catch (const runtime_error &err) {
		throw runtime_error(EXCEPT_DEBUG(err.what()));
	}

	return *this;
}

SMTPServer &SMTPServer::listenServer() {
	auto &config = Global::getConfig();
	auto &logger = this->s_Logger;
	auto &sslSocket = this->s_SSLSocket;
	auto &plainSocket = this->s_PlainSocket;

	int32_t sslPort = config["ports"]["smtp_secure"].asInt();
	int32_t plainPort = config["ports"]["smtp_plain"].asInt();

	// ==============================
	// Uses IPv4 if IPv6 disabled
	// ==============================

	if (!config["ipv6"].asBool()) {
		sslSocket = make_unique<ServerSocket>(ServerSocketAddrType::ServerSocketAddr_IPv4);
		plainSocket = make_unique<ServerSocket>(ServerSocketAddrType::ServerSocketAddr_IPv4);

		sslSocket->queue(250).useSSL(this->s_SSLContext.get()).listenServer(sslPort);
		plainSocket->queue(250).listenServer(plainPort);
		logger << "IPv4 Listening on { SSL: " << sslPort << ", Plain: " << plainPort << " }" << ENDL;
	}

	// ==============================
	// Uses IPv6 if enabled
	// ==============================

	if (config["ipv6"].asBool()) {
		sslSocket = make_unique<ServerSocket>(ServerSocketAddrType::ServerSocketAddr_IPv6);
		plainSocket = make_unique<ServerSocket>(ServerSocketAddrType::ServerSocketAddr_IPv6);

		sslSocket->queue(250).useSSL(this->s_SSLContext.get()).listenServer(sslPort);
		plainSocket->queue(250).listenServer(plainPort);
		logger << "IPv6 Listening on { SSL: " << sslPort << ", Plain: " << plainPort << " }" << ENDL;
	}

	return *this;
}

SMTPServer &SMTPServer::startHandler(const bool newThread) {
	auto &config = Global::getConfig();
	auto &logger = this->s_Logger;

	auto handler = [&](shared_ptr<ClientSocket> client) {
		DEBUG_ONLY(size_t start = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
		shared_ptr<SMTPServerSession> session = make_shared<SMTPServerSession>();
		if (client->usingSSL()) {
			session->setFlag(_SMTP_SERV_SESSION_SSL_FLAG);
		}

		// Creates the logger and prints the initial information to the console
		//  after this we write the first command to the client, which will contain
		//  the greeting.

		Logger clogger("ESMTP[" + client->getPrefix() + ']', LoggerLevel::DEBUG);
		DEBUG_ONLY(clogger << "Client connected" << ENDL);

		try {
			ServerResponse response(SMTPResponseType::SRC_GREETING);
			client->write(response.build());

			size_t itt = 0;
			for (;;) {

				if (++itt > 60) break;
				try {
					string raw = client->readToDelim("\r\n");
					ClientCommand command(raw);

					if (this->handleCommand(client, command, session, clogger)) {
						break;
					}
				} catch (const SMTPOrderException &err) {
					ServerResponse response(SMTPResponseType::SRC_ORDER_ERR, err.what(), nullptr, nullptr);
					client->write(response.build());
				} catch (const SMTPSyntaxException &err) {
					ServerResponse response(SMTPResponseType::SRC_SYNTAX_ERR, err.what(), nullptr, nullptr);
					client->write(response.build());
				} catch (const SMTPInvalidCommand &err) {
					ServerResponse response(SMTPResponseType::SRC_INVALID_COMMAND, err.what(), nullptr, nullptr);
					client->write(response.build());
				} catch (const SMTPFatalException &err) {
					clogger << FATAL << "An error occured: " << err.what() << ENDL << CLASSIC;
					break;
				} catch (const runtime_error &err) {
					clogger << ERROR << "An error occured: " << err.what() << ENDL << CLASSIC;
					break;
				}  catch (const DatabaseException &err) {
					clogger << ERROR << "An database exception occured: '" << err.what() << '\'' << ENDL << CLASSIC;
				} catch (...) {
					clogger << ERROR << "An other error occured, breaking ..." << ENDL << CLASSIC;
					break;
				}
			}
		} catch (const runtime_error &e) {
			clogger << ERROR << "An runtime error occured: " << e.what() << ENDL << CLASSIC;
		} catch (...) {
			clogger << ERROR << "An unknown error occured" << ENDL << CLASSIC;
		}

		#ifdef _SMTP_DEBUG
		size_t end = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		clogger << "Client disconnected, transmission in: " << end - start << " milliseconds" << ENDL;
		#endif
	};

	// Checks if we need to start the acceptor in the same thread
	//  if so we will just do the first one in a different thread, and
	//  put the second one in the same. Else we just put both in
	//  separate ones

	logger << "handlers started" << ENDL;

	this->s_SSLSocket->handler(handler).startAcceptor(true);
	this->s_PlainSocket->handler(handler).startAcceptor(newThread);
	
	return *this;
}

bool SMTPServer::handleCommand(
	shared_ptr<ClientSocket> client, const ClientCommand &command,
	shared_ptr<SMTPServerSession> session, Logger &clogger
) {
	auto *redis = this->s_Redis.get();
	auto *cass = this->s_Cassandra.get();
	auto sendResponse = [&](const SMTPResponseType type) {
		ServerResponse response(type);
		client->write(response.build());
	};

	switch (command.c_CommandType) {
		// ========================================
		// Handles the 'HELP' command
		//
		// Sends information about the server
		// ========================================
		case ClientCommandType::CCT_HELP:
		{
			sendResponse(SMTPResponseType::SRC_HELP_RESP);
			break;
		}
		// ========================================
		// Handles the 'QUIT' command
		//
		// Closes the connection
		// ========================================
		case ClientCommandType::CCT_QUIT:
		{
			// Writes the goodbye message, and closes the connection
			sendResponse(SMTPResponseType::SRC_QUIT_GOODBYE);
			return true;
		}
		// ========================================
		// Handles the 'HELO' command
		//
		// Initializes connection
		// ========================================
		case ClientCommandType::CCT_HELO: {
			if (session->getAction(_SMTP_SERV_PA_HELO)) {
				throw SMTPOrderException("EHLO/HELO already sent.");
			} else if (command.c_Arguments.size() < 1) {
				throw SMTPSyntaxException("Empty HELO command not allowed");
			}

			string clientString = client->getString();
			ServerResponse response(
				SMTPResponseType::SRC_HELO, "",
				reinterpret_cast<const void *>(clientString.c_str()), nullptr
			);
			client->write(response.build());

			session->setAction(_SMTP_SERV_PA_HELO);
			break;
		}
		// ========================================
		// Handles the 'EHLO' command
		//
		// Sends the server capabilities
		// ========================================
		case ClientCommandType::CCT_EHLO: {
			if (session->getAction(_SMTP_SERV_PA_HELO)) {
				throw SMTPOrderException("EHLO/HELO already sent.");
			} else if (command.c_Arguments.size() < 1) {
				throw SMTPSyntaxException("Empty HELO command not allowed");
			}

			string clientString = client->getString();
			ServerResponse response(
				SMTPResponseType::SRC_EHLO,
				"",
				reinterpret_cast<const void *>(clientString.c_str()),
				(client->usingSSL() ? &this->s_SecureServices : &this->s_PlainServices)
			);
			client->write(response.build());

			session->setAction(_SMTP_SERV_PA_HELO);
			break;
		}
		// ========================================
		// Handles the 'STARTTLS' command
		//
		// Upgrades the connection to SSL
		// ========================================
		case ClientCommandType::CCT_START_TLS: {
			if (!session->getAction(_SMTP_SERV_PA_HELO)) {
				throw SMTPOrderException("EHLO/HELO first.");
			} else if (session->getFlag(_SMTP_SERV_SESSION_SSL_FLAG)) {
				throw SMTPOrderException("Connection already secured");
			}

			clogger << "Upgrading to TLS ..." << ENDL;

			sendResponse(SMTPResponseType::SRC_START_TLS);
			client->useSSL(this->s_SSLContext.get()).upgradeAsServer();

			clogger << "Upgraded to TLS" << ENDL;

			session->setFlag(_SMTP_SERV_SESSION_SSL_FLAG);
			session->clearAction(_SMTP_SERV_PA_HELO);
			break;
		}
		// ========================================
		// Handles the 'MAIL FROM' command
		//
		// Sets the senders address
		// ========================================
		case ClientCommandType::CCT_MAIL_FROM: {
			auto &sendingAccount = session->s_SendingAccount;
			EmailAddress from;

			// Checks if the command is allowed, before this
			//  the client needs to introduce herself, and then
			//  the client is also required to have not sent MAIL FROM
			//  before in this session. After this we attempt to parse
			//  the message.

			if (session->getAction(_SMTP_SERV_PA_MAIL_FROM)) {
				throw SMTPOrderException("MAIL FROM already sent");
			} else if (!session->getAction(_SMTP_SERV_PA_HELO)) {
				throw SMTPOrderException("EHLO/HELO first");
			} else if (command.c_Arguments.size() < 1) {
				throw SMTPSyntaxException("MAIL FROM requires arguments.");
			}

			try {
				from.parse(command.c_Arguments[0]);
			} catch (const runtime_error &err) {
				throw SMTPSyntaxException(string("Invalid email address: ") + err.what());
			}

			// Checks if the message is from an different server, or if the message
			//  comes from one of this servers domains. If it is one from our server
			//  we want to require the user to authenticate first.

			string domain = from.getDomain();
			try {
				// Checks if the SU flag is set, if so we we throw an empty query
				//  to indicate that the domain is not local ( while it actually is )
				//  but this tricks the server to allow the messages without auth
				if (session->getFlag(_SMTP_SERV_SESSION_SU)) throw EmptyQuery("");

				// Attempts to find the domain in redis, this check makes sure
				//  that the sending domain is from us, or somebody else. If this
				//  succeeds, and the client is not authenticated, send error.
				
				LocalDomain localDomain = LocalDomain::get(domain, cass, redis);
				if (!session->getFlag(_SMTP_SERV_SESSION_AUTH_FLAG)) {
					throw SMTPOrderException("AUTH first");
				}

				// Since a authenticated user is only allowed to send from his own address, we 
				//  check if the senders domain equals the sending accounts domain. If this
				//  is not true, we tell the client that the transaction is not allowed. If
				//  the transaction is allowed, we set the from local flag. Which indicates
				//  that we possibly need to use the SMTP client.

				if (from.getDomain() != sendingAccount.a_Domain || from.getUsername() != sendingAccount.a_Username) {
					sendResponse(SMTPResponseType::SRC_AUTH_NOT_ALLOWED);
					return true;
				}

				session->setFlag(_SMTP_SERV_SESSION_FROM_LOCAL);
			} catch (const EmptyQuery &e) {}

			// Writes the proceed message to the client, which indicates that everything is fine
			//  and we're ready for further transport
			
			session->setTransformFrom(from);
			ServerResponse response(
				SMTPResponseType::SRC_MAIL_FROM, "",
				reinterpret_cast<const void *>(session->getTransportFrom().e_Address.c_str()),
				nullptr
			);
			client->write(response.build());

			// Sets the from email address in the sessions message, after that
			//  we tell that the mail from action has been used.

			session->setAction(_SMTP_SERV_PA_MAIL_FROM);
			break;
		}
		// ========================================
		// Handles the 'RCPT TO' command
		//
		// Sets the receivers address
		// ========================================
		case ClientCommandType::CCT_RCPT_TO:
		{
			AccountShortcut receivingAccount;
			EmailAddress to;

			// Checks if we're allowed to perform this command, this command
			//  requires the user to sent helo, and performed the mail from command
			//  if this is not the case we will send an error.

			if (!session->getAction(_SMTP_SERV_PA_HELO)) {
				throw SMTPOrderException("EHLO/HELLO first.");
			} else if (!session->getAction(_SMTP_SERV_PA_MAIL_FROM)) {
				throw SMTPOrderException("MAIL FROM first.");
			} else if (command.c_Arguments.size() < 1) {
				throw SMTPSyntaxException("RCPT TO requires arguments.");
			}

			try {
				to.parse(command.c_Arguments[0]);
			} catch (const std::runtime_error &err) {
				throw SMTPSyntaxException(string("Invalid email address: ") + err.what());
			}

			// Checks if the receiver is local, if this is not the case
			//  we will set the relay flag, which will basically trigger the client
			//  later on in the process.

			bool relay = false;
			try {
				LocalDomain localDomain = LocalDomain::get(to.getDomain(), cass, redis);
			} catch (const EmptyQuery &e) {
				if (session->getFlag(_SMTP_SERV_SESSION_FROM_LOCAL)) {
					relay = true;
				}
			}

			// Checks if we're relaying, if this is not the case
			//  we want to check if the target user is local, since we need
			//  to send the message to his mailbox. If the user is not local
			//  we will send an error message, because he is not local.

			if (!relay) {

				// Checks if the user is local, if this is not the case
				//  we will send an error, and return from the current command.

				try {
					receivingAccount = AccountShortcut::find(cass, redis, to.getDomain(), to.getUsername());
				} catch (const EmptyQuery &e) {
					ServerResponse response(
						SMTPResponseType::SRC_REC_NOT_LOCAL, "",
						reinterpret_cast<const void *>(to.e_Address.c_str()), nullptr
					);
					client->write(response.build());

					return false;
				}
			}

			// Since all tests succeeded, and relaying is allowed or the user
			//  is local, we will send the success response, and add the to address
			//  to the vector of receivers

			ServerResponse response(
				SMTPResponseType::SRC_RCPT_TO, "", 
				reinterpret_cast<const void *>(to.e_Address.c_str()), nullptr
			);
			client->write(response.build());

			if (!relay) session->addStorageTask(SMTPServerStorageTask {
				receivingAccount, SMTPServerStorageTarget::StorageTargetIncomming});
			else {
				session->addRelayTask(SMTPServerRelayTask{ to });

				// Checks if we've already set the relay flag, if this is the case
				//  do nothing, when setting the relay flag we also add an storage
				//  task for sent
				if (!session->getFlag(_SMTP_SERV_SESSION_RELAY)) {
					session->setFlag(_SMTP_SERV_SESSION_RELAY);
					session->addStorageTask(SMTPServerStorageTask { session->s_SendingAccount, 
						SMTPServerStorageTarget::StorageTargetSent });
				}
			}

			session->setAction(_SMTP_SERV_PA_RCPT_TO);
			break;
		}
		// ========================================
		// Handles the 'AUTH' command
		//
		// Authenticates the user
		// ========================================
		case ClientCommandType::CCT_AUTH:
		{
			// Checks if we're allowed to perform this command, it may
			//  only be done if not authenticated already, and the client
			//  introduced herself.

			if (session->getAction(_SMTP_SERV_PA_AUTH_PERF)) {
				throw SMTPOrderException("AUTH already done.");						
			} else if (!session->getAction(_SMTP_SERV_PA_HELO)) {
				throw SMTPOrderException("EHLO/HELLO first.");
			} else if (command.c_Arguments.size() < 2) {
				throw SMTPSyntaxException("Specify type, and hash.");
			}

			// =======================================
			// Validates the entry
			//
			// Checks if the password etcetera is
			// - correct, if not throw err
			// =======================================

			// Gets the username and password from the base64 string
			//  this will later be used to check if the passwords match

			string user, pass;
			try {
				tie(user, pass) = getUserAndPassB64(command.c_Arguments[1]);
			} catch (const runtime_error &err) {
				throw SMTPSyntaxException(err.what());
			}

			// Checks if the passwords match, if this is not case we will send an
			//  error message, and close the transmission channel.

			if (!authVerify(redis, cass, user, pass, session->s_SendingAccount)) {
				sendResponse(SMTPResponseType::SRC_AUTH_FAIL);
				return true;
			} else {
				sendResponse(SMTPResponseType::SRC_AUTH_SUCCESS);
			}
			
			session->setAction(_SMTP_SERV_PA_AUTH_PERF);
			session->setFlag(_SMTP_SERV_SESSION_AUTH_FLAG);
			break;
		}
		// ========================================
		// Handles the 'DATA' command
		//
		// Receives and parses the body
		// ========================================
		case ClientCommandType::CCT_DATA:
		{
			// Checks if we're allowed to perform the data command
			//  this may only happen when data not sent already, and the
			//  user performed the previously required steps

			if (session->getAction(_SMTP_SERV_PA_DATA_START)) {
				throw SMTPOrderException("DATA already received.");
			} if (!session->getAction(_SMTP_SERV_PA_HELO)) {
				throw SMTPOrderException("EHLO/HELLO first.");
			} if (!session->getAction(_SMTP_SERV_PA_MAIL_FROM)) {
				throw SMTPOrderException("MAIL FROM first.");
			} if (!session->getAction(_SMTP_SERV_PA_RCPT_TO)) {
				throw SMTPOrderException("RCPT TO first.");
			}

			// Updates the flags to state that the data transmission
			//  has started

			sendResponse(SMTPResponseType::SRC_DATA_START);
			session->setAction(_SMTP_SERV_PA_DATA_START);

			// Handles the data event
			if (SMTP::Server::Handlers::dataHandler(client, session, clogger))
				return true;
			
			break;
		}
		// ========================================
		// Handles the 'SU' command
		//
		// Checks if client is one of our comrades
		// ========================================
		case ClientCommandType::CCT_SU: {
			auto &conf = Global::getConfig();
			auto prefix = client->getPrefix();

			SPF::SPFValidator validator;
			validator.setProtocol(client->getRealProtocol());
			validator.validate(conf["domain"].asString(), prefix);

			if (prefix != "0.0.0.0" && prefix != "127.0.0.1" && validator.getResult().type != SPF::SPFValidatorResultType::ResultTypeAllowed) {
				client->write(ServerResponse(SMTPResponseType::SRC_SU_DENIED).build());
				return true;
			}

			// Writes the greeting to our comrade, and grants
			//  full SMTP access.
			string clientString = client->getString();
			ServerResponse response(
				SMTPResponseType::SRC_SU_ACC, "", 
				reinterpret_cast<const void *>(clientString.c_str()), nullptr
			);
			client->write(response.build());
			session->setFlag(_SMTP_SERV_SESSION_SU);
			break;
		}
		// ====[====================================
		// Handles the 'FCAPA' command
		//
		// Sends that the server implements fannst
		//  smtp extensions
		// ========================================
		case ClientCommandType::CCT_FCAPA: {
			client->write(ServerResponse(SMTPResponseType::SRC_FCAPA_RESP).build());
			break;
		}
		default: {
			throw SMTPInvalidCommand("Invalid command");
		}
	}

	return false;
}
