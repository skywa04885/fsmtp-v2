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
	P3Server::P3Server():
		s_Logger("POP3", LoggerLevel::INFO)
	{
		// Sets the capabilities
		this->s_Capabilities.push_back(POP3Capability{
			"EXPIRE",
			{"NEVER"}
		});
		this->s_Capabilities.push_back(POP3Capability{

			"LOGIN-DELAY",
			{"0"}
		});
		this->s_Capabilities.push_back(POP3Capability{"STLS", {}});
		this->s_Capabilities.push_back(POP3Capability{"USER", {}});
		this->s_Capabilities.push_back(POP3Capability{"TOP", {}});
		this->s_Capabilities.push_back(POP3Capability{"UIDL", {}});
		this->s_Capabilities.push_back(POP3Capability{"RESP-CODES", {}});
		this->s_Capabilities.push_back(POP3Capability{"AUTH-RESP-CODE", {}});
		this->s_Capabilities.push_back(POP3Capability{"IMPLEMENTATION", {}});	
	}

	P3Server &P3Server::connectDatabases() {
		auto &logger = this->s_Logger;
		auto &redis = this->s_Redis;
		auto &cassandra = this->s_Cassandra;

		cassandra = Global::getCassandra();
		logger << _BASH_SUCCESS_MARK << "Connected to cassandra" << ENDL;

		redis = Global::getRedis();
		logger << _BASH_SUCCESS_MARK << "Connected to redis" << ENDL;

		return *this;
	}

	P3Server &P3Server::createContext() {
		auto &config = Global::getConfig();
		auto &sslCtx = this->s_SSLContext;

		const char *pass = config["ssl_pass"].asCString();
		const char *key = config["ssl_key"].asCString();
		const char *cert = config["ssl_cert"].asCString();

		sslCtx = make_unique<SSLContext>();
		sslCtx->method(SSLv23_server_method()).password(pass).read(key, cert);

		return *this;
	}

	P3Server &P3Server::listenServer() {
		auto &sslCtx = this->s_SSLContext;
		auto &logger = this->s_Logger;
		auto &sslSocket = this->s_SSLSocket;
		auto &plainSocket = this->s_PlainSocket;

		sslSocket = make_unique<ServerSocket>();
		plainSocket = make_unique<ServerSocket>();

		sslSocket->queue(250).useSSL(sslCtx.get()).listenServer(995);
		plainSocket->queue(250).listenServer(110);

		logger << "listening on SSL:995, PLAIN:110" << ENDL;

		return *this;
	}

	P3Server &P3Server::startHandler(const bool newThread) {
		auto *cassandra = this->s_Cassandra.get();

		auto handler = [&](shared_ptr<ClientSocket> client) {
			P3ServerSession session;
			Logger clogger("POP3" + client->getPrefix(), LoggerLevel::DEBUG);
			clogger << "Client connected" << ENDL;

			client->write(P3Response(
				true, POP3ResponseType::PRT_GREETING, 
				"", nullptr, nullptr,
				reinterpret_cast<void *>(client->getAddress())
			).build());
			

			// Performs the infinite read/write loop.. In here we parse
			//  the client command and send the according response. 
			//  If anything goes wrong we print the error, and exit
			//  on an fatal exception.		
		
			for (;;) {
				try {
					P3Command command(client->readToDelim("\r\n"));

					if (P3Server::handleCommand(client, command, session, clogger)) {
						break;
					}
				} catch (const P3FatalException &e) {
					clogger << FATAL << "An fatal exception occured, message: " << e.what() << ENDL;
					break;
				} catch (const P3SyntaxException &e) {
					client->write(P3Response(
						false, POP3ResponseType::PRT_SYNTAX_ERROR, "",
						nullptr, nullptr, reinterpret_cast<const void *>(e.what())
					).build());
				} catch (const P3InvalidCommand &e) {
					client->write(P3Response(
						false, POP3ResponseType::PRT_COMMAND_INVALID, "",
						nullptr, nullptr, reinterpret_cast<const void *>(e.what())
					).build());
				} catch (const P3OrderException &e) {
					client->write(P3Response(
						false, POP3ResponseType::PRT_ORDER_ERROR, "",
						nullptr, nullptr, reinterpret_cast<const void *>(e.what())
					).build());
				}  catch (const runtime_error &e) {
					clogger << ERROR << "An runtime error occured, message: " << e.what() << ENDL << CLASSIC;
				} catch (const exception &e) {
					clogger << ERROR << "An error occured, message: " << e.what() << ENDL << CLASSIC;
				} catch (...) {
					clogger << ERROR << "An error occured, message: unknown." << ENDL << CLASSIC;
				}
			}


			// Burries the death after the session clossed
			if (session.s_Graveyard.size() > 0) {
				DEBUG_ONLY(clogger << WARN << "Deleting " << session.s_Graveyard.size() << " emails !" << ENDL);
				for (const std::size_t i : session.s_Graveyard)
				{
					// Deletes the the raw email, shortcut and parsed email
					const CassUuid &uuid = std::get<0>(session.s_References[i]);
					const int64_t &bucket = std::get<2>(session.s_References[i]);

					// Deletes the emails
					EmailShortcut::deleteOne(
						cassandra,
						session.s_Account.a_Domain,
						session.s_Account.a_UUID,
						uuid
					);
					FullEmail::deleteOne(
						cassandra,
						session.s_Account.a_Domain,
						session.s_Account.a_UUID,
						uuid,
						bucket
					);
					RawEmail::deleteOne(
						cassandra,
						session.s_Account.a_Domain,
						session.s_Account.a_UUID,
						uuid,
						bucket
					);
				}
			}

			clogger << "Client disconnected" << ENDL;
		};

		if (!newThread) {
			this->s_SSLSocket->handler(handler).startAcceptor(true);
			this->s_PlainSocket->handler(handler).startAcceptor(false);
		} else {
			this->s_SSLSocket->handler(handler).startAcceptor(true);
			this->s_PlainSocket->handler(handler).startAcceptor(true);
		}

		return *this;
	}

	
	bool P3Server::handleCommand(
		shared_ptr<ClientSocket> client, P3Command &command,
		P3ServerSession &session, Logger &clogger
	) {
		auto &conf = Global::getConfig();
		auto *cassandra = this->s_Cassandra.get();
		auto *redis = this->s_Redis.get();

		switch (command.c_Type) {
			case POP3CommandType::PCT_IMPLEMENTATION: {
				client->write(P3Response(true, POP3ResponseType::PRT_IMPLEMENTATION).build());
				break;
			}
			// =========================================
			// Handles the 'TOP' command
			//
			// Gets the headers and peeks the message
			// =========================================
			case POP3CommandType::PCT_TOP:
			{
				// Checks if we're authenticated
				if (!session.getFlag(_P3_SERVER_SESS_FLAG_AUTH))
					throw P3OrderException("USER/PASS First.");

				// Checks if there are enough arguments
				if (command.c_Args.size() < 2)
					throw P3SyntaxException("TOP requires two arguments.");

				// Parses the arguments, and throws syntax error when
				// - wrong entered
				std::size_t i, line;
				try {
					i = stol(command.c_Args[0]);
					--i;
				} catch (const std::invalid_argument& e)
				{
					throw P3SyntaxException("TOP invalid index");
				}

				try {
					line = stol(command.c_Args[1]);
				} catch (const std::invalid_argument& e) {
					throw P3SyntaxException("TOP invalid line");
				}

				// Gets the UUID from the specified message, and then
				// - query's the raw message
				const CassUuid &uuid = std::get<0>(session.s_References[i]);
				RawEmail raw = RawEmail::get(
					cassandra, 
					session.s_Account.a_Domain,
					session.s_Account.a_UUID,
					uuid,
					std::get<2>(session.s_References[i])
				);

				// Prepares the email contents, and splits the message
				// - into the headers and body
				std::string headers, body;
				MIME::splitHeadersAndBody(raw.e_Content, headers, body);

				// Checks how we should return the data
				if (line > 0)
				{
					i = 0;
					headers += "\r\n";
					std::stringstream stream(body);
					std::string token;
					while (std::getline(stream, token, '\n'))
					{
						if (token[token.size() - 1] == '\r') token.pop_back();
						if (i++ > line) break;
						headers += token;
						headers += "\r\n";
					}
				}

				if (headers[headers.size() - 1] != '\n' && headers[headers.size() - 2] != '\r')
					headers += "\r\n";
				headers += ".\r\n";


				// Writes the response
				client->write(P3Response(
					true,
					POP3ResponseType::PRT_TOP,
					"",
					nullptr, nullptr,
					reinterpret_cast<void *>(&std::get<1>(session.s_References[i]))
				).build());
				client->write(headers);

				break;
			}
			// =========================================
			// Handles the 'STLS' command
			//
			// Secures the connection with the client
			// - using SSL
			// =========================================
			case POP3CommandType::PCT_STLS:
			{
				client->write(P3Response(
					true,
					POP3ResponseType::PRT_STLS_START,
					"", nullptr, nullptr,
					reinterpret_cast<void *>(client->getAddress())
				).build());
				client->useSSL(this->s_SSLContext.get()).upgradeAsServer();
				break;
			}
			// =========================================
			// Handles the 'QUIT' command
			//
			// Closes the connection with the client
			// =========================================
			case POP3CommandType::PCT_QUIT:
			{
				client->write(P3Response(true, PRT_QUIT).build());
				return true;
			}
			// =========================================
			// Handles the 'PASS' command
			//
			// The second phase of the auth process
			// - where the password is verified
			// =========================================
			case POP3CommandType::PCT_PASS:
			{
				// Checks if we're allowed to perform this command
				// - this is only possible if we're not authenticated
				// - and the client has sent the password
				if (session.getFlag(_P3_SERVER_SESS_FLAG_AUTH))
					throw P3OrderException("Client already authenticated.");
				if (!session.getAction(_P3_SERVER_SESS_ACTION_USER))
					throw P3OrderException("USER first.");

				// Checks if the command is not empty
				if (command.c_Args.size() < 1) {
					throw P3SyntaxException("PASS requires one argument");
				}

				// Gets the user password from the database
				std::string publicKey, password;
				try {
					std::tie(password, publicKey) = Account::getPassAndPublicKey(
						cassandra,
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
					client->write(P3Response(
						false,
						POP3ResponseType::PRT_AUTH_FAIL,
						"",
						nullptr, nullptr,
						reinterpret_cast<void *>(const_cast<char *>(error.c_str()))
					).build());
					break;
				}

				// Compares the passwords to check if it realy is the user
				if (!passwordVerify(command.c_Args[0], password))
				{
					client->write(P3Response(
						false,
						POP3ResponseType::PRT_AUTH_FAIL,
						"",
						nullptr, nullptr,
						reinterpret_cast<void *>(const_cast<char *>("invalid password"))
					).build());
					break;
				}

				// Gets the emails available
				session.s_References = EmailShortcut::gatherAllReferencesWithSize(
					cassandra,
					0,
					120,
					session.s_Account.a_Domain,
					"INBOX",
					session.s_Account.a_UUID
				);

				// Stores the password in the session, sets the flag and sends
				// - the continue response
				session.s_Pass = command.c_Args[0];
				session.setAction(_P3_SERVER_SESS_ACTION_PASS);
				session.setFlag(_P3_SERVER_SESS_FLAG_AUTH);
				client->write(P3Response(true, POP3ResponseType::PRT_AUTH_SUCCESS).build());
				break;
			}
			// =========================================
			// Handles the 'USER' command
			//
			// The first stage of the login process
			// - after this the password is needed
			// =========================================
			case POP3CommandType::PCT_USER:
			{
				EmailAddress address;

				// Checks if we're allowed to perform this command,
				// - this may only happen when the user has not
				// - authenticated yet
				if (session.getFlag(_P3_SERVER_SESS_FLAG_AUTH))
					throw P3OrderException("Client already authenticated.");

				// Checks if the command is not empty
				if (command.c_Args.size() < 1)
				{
					throw P3SyntaxException("USER requires one argument");
				}

				// Checks if there is an domain, if not
				// - ignore this step and append the default one
				if (command.c_Args[0].find_first_of('@') != std::string::npos) {
					address.parse(command.c_Args[0]);

					// Searches the database for the local domain, so we
					// - know if the users domain is on this server
					try {
						LocalDomain domain = LocalDomain::findRedis(address.getDomain(), redis);
					} catch (const EmptyQuery &e) {
						// Constructs the error
						std::string error = "domain [";
						error += address.getDomain();
						error += "] was not found on this server.";

						// Sends the error response
						client->write(P3Response(
							false,
							POP3ResponseType::PRT_AUTH_FAIL,
							"",
							nullptr, nullptr,
							reinterpret_cast<void *>(const_cast<char *>(error.c_str()))
						).build());
					}
				} else {
					command.c_Args[0] += '@';
					command.c_Args[0] += conf["domain"].asCString();
					address.parse(command.c_Args[0]);
				}

				// Checks if the username with the domain is found
				// - in our redis database
				try {
					session.s_Account = AccountShortcut::findRedis(redis, address.getDomain(), address.getUsername());
				} catch (const EmptyQuery &e) {
					// Constructs the error
					std::string error = "user [";
					error += address.e_Address;
					error += "] was not found on this server.";

					// Sends the error response
					client->write(P3Response(
						false,
						POP3ResponseType::PRT_AUTH_FAIL,
						"",
						nullptr, nullptr,
						reinterpret_cast<void *>(const_cast<char *>(error.c_str()))
					).build());
					
					return false;
				}

				// Prints the debug message
				#ifdef _SMTP_DEBUG
				char uuidBuffer[64];
				cass_uuid_string(session.s_Account.a_UUID, uuidBuffer);
				clogger << DEBUG << "Identified as " << session.s_Account.a_Bucket << '@' << uuidBuffer << ENDL << CLASSIC;
				#endif

				// Stores the user in the session, sets the flag and sends
				// - the continue response
				session.s_User = command.c_Args[0];
				session.setAction(_P3_SERVER_SESS_ACTION_USER);
				client->write(P3Response(true, POP3ResponseType::PRT_USER_DONE).build());
				break;
			}
			// =========================================
			// Handles the 'CAPA' command
			//
			// Lists the servers capabilities
			// =========================================
			case POP3CommandType::PCT_CAPA:
			{
				// Sends the list of capabilities
				client->write(P3Response(
					true,
					POP3ResponseType::PRT_CAPA,
					"",
					&this->s_Capabilities,
					nullptr, nullptr
				).buildCapabilities());
				break;
			}
			// =========================================
			// Handles the 'STATI' command
			//
			// Gets the status, the count of emails
			// - and their summed size
			// =========================================
			case POP3CommandType::PCT_STAT:
			{
				// Checks if we're authenticated
				if (!session.getFlag(_P3_SERVER_SESS_FLAG_AUTH))
					throw P3OrderException("USER/PASS First.");

				// Gets the data and builds the response
				int64_t octets = 0;
				for (const std::tuple<CassUuid, int64_t, int64_t> &pair : session.s_References)
					octets += std::get<1>(pair);

				std::string response = std::to_string(session.s_References.size());
				response += ' ';
				response += std::to_string(octets);

				// Sends the stat response
				client->write(P3Response(
					true,
					POP3ResponseType::PRT_STAT,
					response, nullptr, nullptr, nullptr
				).build());
				break;
			}
			// =========================================
			// Handles the 'UIDL' command
			//
			// Lists the messages with the number version
			// - of their timeuuid
			// =========================================
			case POP3CommandType::PCT_UIDL:
			{
				// Checks if we're authenticated
				if (!session.getFlag(_P3_SERVER_SESS_FLAG_AUTH))
					throw P3OrderException("USER/PASS First.");

				std::vector<POP3ListElement> list = {};

				std::size_t i = 0;
				for (const std::tuple<CassUuid, int64_t, int64_t> &ref : session.s_References)
					list.push_back(POP3ListElement{
						++i,
						std::to_string(cass_uuid_timestamp(std::get<0>(ref)))
					});

				// Sends the response
				client->write(P3Response(
					true,
					POP3ResponseType::PRT_UIDL,
					"", nullptr, &list, nullptr
				).buildList());

				break;
			}
			// =========================================
			// Handles the 'LIST' command
			//
			// this will list all the messages, with the
			// - assigned size in octets
			// =========================================
			case POP3CommandType::PCT_LIST:
			{
				// Checks if we're authenticated
				if (!session.getFlag(_P3_SERVER_SESS_FLAG_AUTH))
					throw P3OrderException("USER/PASS First.");

				std::vector<POP3ListElement> list = {};

				std::size_t i = 0;
				for (const std::tuple<CassUuid, int64_t, int64_t> &ref : session.s_References)
					list.push_back(POP3ListElement{
						++i,
						std::to_string(std::get<1>(ref))
					});

				// Sends the response
				client->write(P3Response(
					true,
					POP3ResponseType::PRT_LIST,
					"", nullptr, &list, nullptr
				).buildList());

				break;
			}
			// =========================================
			// Handles the 'RETR' command
			//
			// this will send an email back to the client
			// =========================================
			case POP3CommandType::PCT_RETR:
			{
				// Checks if we're authenticated
				if (!session.getFlag(_P3_SERVER_SESS_FLAG_AUTH))
					throw P3OrderException("USER/PASS First.");

				// Checks if there is an argument
				if (command.c_Args.size() < 1)
					throw P3SyntaxException("RETR requires one argument.");

				// Tries to parse the argument, and checks if the index is too
				// - large by comparing it to the vector
				std::size_t i;
				try {
					i = std::stol(command.c_Args[0]);
					--i;
				} catch (const std::invalid_argument &e) {
					throw P3SyntaxException("Invalid index for RETR");
				}
				
				if (i > session.s_References.size()) {
					std::string error = "Max index ";
					error += std::to_string(session.s_References.size());
					throw P3SyntaxException(error);
				}

				// Gets the UUID from the specified message, and then
				// - query's the raw message
				const CassUuid &uuid = std::get<0>(session.s_References[i]);
				RawEmail raw = RawEmail::get(
					cassandra, 
					session.s_Account.a_Domain,
					session.s_Account.a_UUID,
					uuid,
					std::get<2>(session.s_References[i])
				);

				// Prepares the email contents
				if (raw.e_Content[raw.e_Content.size() - 1] != '\n' && 
						raw.e_Content[raw.e_Content.size() - 2] != '\r') {
					raw.e_Content += "\r\n";
				}

				raw.e_Content += ".\r\n";

				// Sends the email contents
				client->write(P3Response(
					true,
					POP3ResponseType::PRT_RETR,
					"",
					nullptr, nullptr,
					reinterpret_cast<void *>(&std::get<1>(session.s_References[i]))
				).build());
				client->write(raw.e_Content);

				break;
			}
			// =========================================
			// Handles the 'RSET' command
			//
			// Resets the email
			// =========================================
			case POP3CommandType::PCT_RSET:
			{
				session.s_Graveyard.clear();
				client->write(P3Response(true, POP3ResponseType::PRT_RSET).build());
				break;
			}
			// =========================================
			// Handles the 'DELE' command
			//
			// this will delete an email from the server
			// =========================================
			case POP3CommandType::PCT_DELE: {
				// Checks if we're authenticated
				if (!session.getFlag(_P3_SERVER_SESS_FLAG_AUTH)) {
					throw P3OrderException("USER/PASS First.");
				}

				// Checks if there are any arguments, if not throw
				// - syntax error
				if (command.c_Args.size() < 1) {
					throw P3SyntaxException("DELE requires one argument.");
				}

				// Parses the index, if it is too large send error
				std::size_t i;
				try {
					i = std::stol(command.c_Args[0]);
					--i;
				} catch (const std::invalid_argument &e) {
					throw P3SyntaxException("Invalid index for DELE");
				}

				if (i > session.s_References.size()) {
					std::string error = "Max index ";
					error += std::to_string(session.s_References.size());
					throw P3SyntaxException(error);
				}

				// Pushes the index to the graveyard
				session.s_Graveyard.push_back(i);

				// Sends the confirmation
				client->write(P3Response(true, POP3ResponseType::PRT_DELE_SUCCESS).build());
				break;
			}
			case POP3CommandType::PCT_UNKNOWN:
			{
				throw P3InvalidCommand("Command not found");
				break;
			}
		}

		return false;
	}
}
