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

#include "SMTPClient.src.h"

namespace FSMTP::Mailer::Client {
	void __smtpClientServerLog(Logger &logger, const SMTPClientServer &server) {
		logger << DEBUG;
		
		logger << "SMTPClientServer {" << ENDL;
		logger << "\tAdress: " << server.address << ENDL;
		logger << "\tProtocol: " << Networking::IP::__protocolString(server.protocol) << ENDL;
		logger << "}" << ENDL;

		logger << CLASSIC;
	}

	SMTPClient::SMTPClient(bool s_Silent): s_Logger("SMTPClient", LoggerLevel::INFO), s_ErrorCount(0), s_ErrorLog(json::array()) {
		if (!s_Silent) this->s_Logger << "SMTPClient initialized !" << ENDL;
	}

	vector<SMTPClientServer> SMTPClient::getServers(const string &domain) {
		DEBUG_ONLY(auto &logger = this->s_Logger);
		auto &conf = Global::getConfig();

		vector<SMTPClientServer> servers = {};
		DNS::Resolver resolver;

		// Parses the MX records, so we can later start resolving
		//  the IPv4 / IPv6 addresses of them
		vector<DNS::RR> records = resolver.query(domain.c_str(), ns_t_mx).initParse().getRecords();

		// If IPv6 enabled, first try to resolve all the IPv6 addresses
		//  if this ends up as an empty array, then we will resolve IPv4
		if (conf["ipv6"].asBool()) {
			DEBUG_ONLY(logger << DEBUG << "IPv6 enabled, attempting AF_INET6 resolving .." << ENDL << CLASSIC);

			// Loops over the records, and attempts AF_INET6 address resolving
			//  this will override the default AF_INET
			for_each(records.begin(), records.end(), [&](const DNS::RR &rr) {
				try {
					servers.push_back(SMTPClientServer {
						DNS::resolveHostname(rr.getData(), AF_INET6),
						Networking::IP::Protocol::Protocol_IPv6
					});
				} catch (...) {}
			});

			DEBUG_ONLY(logger << DEBUG << "Resolved " << servers.size() << " IPv6 addresses .." << ENDL << CLASSIC);
		}

		// Checks if the vector is empty, this will be when IPv6 is disabled
		//  or there are no IPv6 addresses available
		if (servers.size() <= 0) {
			// Prints the debug messages, to indicate if any IPv6 addresses have been found
			//  if not, we will print an different message which indicates the failure
			if (conf["ipv6"].asBool()) {
				DEBUG_ONLY(logger << DEBUG << "IPv6 resolving failed, attempting AF_INET .." << ENDL << CLASSIC);
			} else {
				DEBUG_ONLY(logger << DEBUG << "IPv6 disabled, attempting AF_INET resolving .." << ENDL << CLASSIC);
			}

			// Starts looping over the records, and attempting the AF_INET address
			//  resolving method
			for_each(records.begin(), records.end(), [&](const DNS::RR &rr) {
				try {
					servers.push_back(SMTPClientServer {
						DNS::resolveHostname(rr.getData(), AF_INET),
						Networking::IP::Protocol::Protocol_IPv4
					});
				} catch (...) {}
			});
		}

		// Checks if we're in debug, and prints the servers if it is the case
		//  just to make sure everything is working properly, after which we
		//  return the list
		DEBUG_ONLY(logger << DEBUG << "Resolved " << servers.size() << " servers !" << ENDL << CLASSIC);
		DEBUG_ONLY(__smtpClientsServerLog<vector<SMTPClientServer>>(logger, servers));

		return servers;
	}

	SMTPClient &SMTPClient::sign(const string &message) {
		auto &conf = Global::getConfig();

		// Gets the current time so that we can set the signing
		//  expire and sign date
		int64_t now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();

		// Prepares the signer, by setting the default parameters
		//  which will be used in the signing process
		DKIM::DKIMSigner sign;
		sign.setDomain(conf["dkim"]["domain"].asString())
			.setKeySelector(conf["dkim"]["keyselector"].asString())
			.setPrivateKeyPath(conf["dkim"]["dkim_private"].asString())
			.setSignTime(now)
			.setExpireTime(now + (1000 * 60 * 60))
			.setAlgoPair(DKIM::DKIMHeaderCanonAlgPair::RelaxedRelaxed)
			.setSignAlgo(DKIM::DKIMHeaderAlgorithm::HeaderAlgoritmRSA_SHA256)
			.headerFilterPush("subject").headerFilterPush("from")
			.headerFilterPush("to").headerFilterPush("date")
			.headerFilterPush("mime-version").headerFilterPush("message-id");

		// Signs the email, and stores it inside the transport message
		this->s_TransportMessage = sign.sign(message).getResult();
		return *this;
	}

	SMTPClient &SMTPClient::prepare(
		const vector<EmailAddress> to,
		const vector<EmailAddress> from,
		const string &message
	) {
		if (!s_Silent) this->s_Logger << "Voorbereiden ..." << ENDL;

		this->sign(message);
		this->s_MailFrom = from[0];

		this->configureRecipients(to);

		return *this;
	}

	SMTPClient &SMTPClient::reset() {
		this->s_ErrorLog.clear();
		this->s_Targets.clear();
		this->s_ErrorCount = 0;
		return *this;
	}

	SMTPClient &SMTPClient::configureRecipients(const vector<EmailAddress> &addresses) {
		// Loops over the recipients, and for each one we get the domain
		//  and resolve the available servers for it
		for_each(addresses.begin(), addresses.end(), [&](const EmailAddress &address) {
			try {
				this->s_Targets.push_back(SMTPClientTarget {
					this->getServers(address.getDomain()),
					address
				});
			} catch(const runtime_error &e) {
				this->s_Logger << ERROR << "Could not resolve records: " << e.what() << ENDL << CLASSIC;
				this->addError(address.toString(), string("Resolver exception: '") + e.what() + '\'');	
			}
		});

		return *this;
	}

	SMTPClient &SMTPClient::prepare(MailComposerConfig &config) {
		if (!s_Silent) this->s_Logger << "Voorbereiden ..." << ENDL;

		this->sign(compose(config));
		this->s_MailFrom = config.m_From[0];

		this->configureRecipients(config.m_To);
		return *this;
	}

	SMTPClient &SMTPClient::addError(
		const string &address,
		const string &message
	) {

		json error;
		error["address"] = escapeHTML(address);
		error["message"] = message;
		this->s_ErrorLog.push_back(error);

		s_ErrorCount++;
		return *this;
	}

	SMTPClient &SMTPClient::beSocial(void) {
		unique_ptr<SSLContext> sslCtx = make_unique<SSLContext>();
		sslCtx->method(SSLv23_method()).justCreate();
		auto &conf = Global::getConfig();
		string domain = conf["smtp"]["client"]["domain"].asCString();

		if (!this->s_Silent) this->s_Logger << "Versturen naar "
			<< this->s_Targets.size() << " clients .." << ENDL;

		// Loops over the targets and creates the transmission
		// - sockets
		for (SMTPClientTarget &target : this->s_Targets) {
			auto &server = target.servers[0];
			int32_t port = conf["smtp"]["client"]["mailer_port"].asInt();

			// ===============================
			// Connects to the server
			// ===============================

			// Creates the client socket class, with the protcol
			//  type specified in the server
			ClientSocket client(server.protocol);

			// Attempts to connect to the server, if this fails
			//  we will add an connection error to the log
			try {
				client.connectAsClient(server.address.c_str(), port);
				
				if (!this->s_Silent)
					this->s_Logger << "Connected to: [" << server.address << "]:" << port << ENDL;
			} catch (const runtime_error &err) {
				this->addError(target.address.toString(),
					string("Failed to connect to [") + server.address + "]:" + to_string(port) + ", error: " + err.what());	
				if (!this->s_Silent)
					this->s_Logger << ERROR << "Kon niet verbinden met server: " << err.what() << ENDL;
				continue;
			}

			auto sendCommand = [&](const ClientCommandType commandType,	const std::vector<std::string> &args) {
				ClientCommand command(commandType, args);
				client.write(command.build());
			};

			// ===============================
			// Starts sending the message
			// 
			// Starts sending the message over
			// - the socket
			// ===============================

			SMTPClientSession session;

			bool _run = true;
			bool _performParse = true;

			while (_run) {
				int32_t code;
				string args;

				// ===================================
				// Receives the default command
				//
				// Receives the data from the server
				// - and stores the code and args
				// ===================================

				if (_performParse) {
					try {
						// Reads all the command
						string raw;
						for (size_t i = 0; i < 400; ++i) {
							string line = client.readToDelim("\r\n");

							// Checks if we need to quit after this round, this is important
							//  since the dash will be removed
							bool shouldQuit = false;
							if (line[3] == ' ') shouldQuit = true;

							// Appends the line, and removes the dash or the number in total
							//  if required
							if (i > 0) raw += line.substr(4);
							else {
								if (line[3] == '-') line[3] = ' ';
								raw += line;
							}

							// Quits the loop if required
							if (shouldQuit) break;
						}

						// Parses the response, and prints it to the console
						tie(code, args) = ServerResponse::parseResponse(raw);
						DEBUG_ONLY(this->printReceived(code, args));
					} catch(const runtime_error &e) {
						_run = false;
					} catch (const invalid_argument &e) {
						this->s_Logger << ERROR << "Invalid response received" << ENDL << CLASSIC;
						_run = false;
					}
				} else {
					_performParse = true;
				}

				// ===================================
				// Acts based on the code
				//
				// Checks the code and acts based
				// - upon it
				// ===================================

				switch (code) {
					case 221: {
						if (session.getAction(_SCS_ACTION_QUIT)) {
							_run = false;
							DEBUG_ONLY(this->s_Logger << DEBUG << "Closing transmission channel ..." << ENDL << CLASSIC);
							continue;
						}
					}

					case 250: {
						// Checks if we need to perform start tls, this
						// - is only possible if hello is send and esmtp is
						// - enabled
						if (
							session.getAction(_SCS_ACTION_HELO) &&
							!session.getAction(_SCS_ACTION_START_TLS) &&
							session.getFlag(_SCS_FLAG_ESMTP)
						) {
							session.setAction(_SCS_ACTION_START_TLS);

							// Sends the STARTTLS command
							sendCommand(ClientCommandType::CCT_START_TLS, {});
							DEBUG_ONLY(this->printSent("STARTTLS"));
							break;
						}

						// Checks if we need to perform mail form, this can be when
						// - we upgraded the ESMTP connection or when we just
						// - send hello, and the server was not ESMTP
						if (
							(
								session.getAction(_SCS_ACTION_HELO) &&
								!session.getFlag(_SCS_FLAG_ESMTP)
							) ||
							(
								session.getAction(_SCS_ACTION_START_TLS_FINISHED) &&
								session.getFlag(_SCS_FLAG_ESMTP) &&
								!session.getAction(_SCS_ACTION_MAIL_FROM)
							)
						) {
							session.setAction(_SCS_ACTION_MAIL_FROM);

							sendCommand(ClientCommandType::CCT_MAIL_FROM, {
								"<" + this->s_MailFrom.e_Address + ">"
							});
							DEBUG_ONLY(this->printSent("MAIL FROM: [sender]"));
							break;
						}

						// Checks if we need to perform the rcpt to, this can only
						// - be when we already sent the mail from, we do not need
						// - to perform esmtp check, since the mail from flag already
						// - has some checks, so we only have to check if it was not 
						// - performed
						if (
							session.getAction(_SCS_ACTION_MAIL_FROM) &&
							!session.getAction(_SCS_ACTION_RCPT_TO)
						) {
							session.setAction(_SCS_ACTION_RCPT_TO);

							sendCommand(ClientCommandType::CCT_RCPT_TO, {
								"<" + target.address.e_Address + ">"
							});
							DEBUG_ONLY(this->printSent("RCPT TO: [target]"));
							break;
						}

						// Checks if we need to send the data segment, this is only
						// - possible if the rcpt to is already sent
						if (
							session.getAction(_SCS_ACTION_RCPT_TO) &&
							!session.getAction(_SCS_ACTION_DATA_START) &&
							!session.getAction(_SCS_ACTION_DATA_END)
						) {
							session.setAction(_SCS_ACTION_DATA_START);

							// Sends the command
							sendCommand(ClientCommandType::CCT_DATA, {});
							continue;
						}
					}

					case 354:
					{
						// Checks if we need to send the data body, this is only possible
						// - if the data start was sent
						if (
							session.getAction(_SCS_ACTION_DATA_START) &&
							!session.getAction(_SCS_ACTION_DATA_END)
						) {
							session.setAction(_SCS_ACTION_DATA_END);

							// Writes the body and the "\r\n.\r\n" after that
							client.write(this->s_TransportMessage);
							client.write("\r\n.\r\n");
							DEBUG_ONLY(this->printSent("DATA [BODY]"));
							continue;
						}
					}

					case 220: {
						// Checks if we need to send quit, this is only possible if the
						// - message body was sent
						if (
							session.getAction(_SCS_ACTION_DATA_END) &&
							!session.getAction(_SCS_ACTION_QUIT)
						) {
							session.setAction(_SCS_ACTION_QUIT);

							// Writes the command
							sendCommand(ClientCommandType::CCT_QUIT, {});
							continue;
						}


						// Checks if we need to upgrade the socket
						// - this is only possible if start tls command is sent
						// - and the server allwos esmtp, and if we have not done
						// - this already
						if (
							session.getAction(_SCS_ACTION_START_TLS) &&
							!session.getAction(_SCS_ACTION_START_TLS_FINISHED) &&
							session.getFlag(_SCS_FLAG_ESMTP)
						) {
							session.setAction(_SCS_ACTION_START_TLS_FINISHED);

							// Upgrades the socket
							DEBUG_ONLY(this->s_Logger << DEBUG << "Omzetten naar TLS verbinding ..." << ENDL << CLASSIC);
							try {
								client.useSSL(sslCtx.get());
								client.upgradeAsClient();
								DEBUG_ONLY(this->s_Logger << DEBUG << "Verbinding omgezet !" << ENDL << CLASSIC);								
							} catch (const runtime_error &err) {
								string message = "SSL Initialization error: ";
								message += err.what();
								this->addError(target.address.toString(), message);
								
								_run = false;
								continue;
							}


							// Sets the HELO flag to false, so that the code will fall throug
							// - and performs the operations again, like EMSPT check and opti
							// - ions initialization
							session.clearAction(_SCS_ACTION_HELO);
						}

						// Checks if it is the initial hello, and performs
						// - the ESMTP check, this will allow later functions
						// - to adapt their commands to the server
						if (
							!session.getAction(_SCS_ACTION_HELO) &&
							!session.getFlag(_SCS_FLAG_ESMTP)
						) {
							if (args.find("ESMTP") != string::npos)
								session.setFlag(_SCS_FLAG_ESMTP);
						}

						// Checks if we need to send the initial hello message
						// - using the ESMTP protocol
						if (
							!session.getAction(_SCS_ACTION_HELO) &&
							session.getFlag(_SCS_FLAG_ESMTP)
						) { // -> Initial HELO command
							// Checks if the target message is the same domain, if so we send the SU command
							//  to the server to request access
							try {
								if (target.address.getDomain() == conf["domain"].asCString()) {
									sendCommand(ClientCommandType::CCT_FCAPA, {});
									DEBUG_ONLY(this->printSent("FCAPA"));
									int32_t code;
									string args;

									tie(code, args) = ServerResponse::parseResponse(client.readToDelim("\r\n"));
									DEBUG_ONLY(this->printReceived(code, args));

									if (code == 601) {
										// Sends the SU command, which most likely will work
										sendCommand(ClientCommandType::CCT_SU, {});
										DEBUG_ONLY(this->printSent("SU"));

										// Gets the response code and arguments, after which we perform the debug print
										tie(code, args) = ServerResponse::parseResponse(client.readToDelim("\r\n"));
										DEBUG_ONLY(this->printReceived(code, args));

										// Checks if the server granted us the permissions to transmit from the same domain
										//  this is again (onmly with fsmtp-v2 servers)
										if (code == 600) DEBUG_ONLY(this->s_Logger << DEBUG << "SU Permissions granted" << ENDL << CLASSIC);
										else if (code == 651) throw runtime_error("Access denied to server, check SPF");
										else throw runtime_error("SU Failed");
									} else throw runtime_error("FCAPA Not supported");
								}
							} catch (const runtime_error &err) {
								this->addError(target.address.toString(), string("FSMTP-V2 Error: ") + err.what());	
								_run = false;
								continue;
							} catch (...) {
								this->addError(target.address.toString(), "An unknown FSMTP-V2 Error occured");	
								_run = false;
								continue;
							}

							session.setAction(_SCS_ACTION_HELO);

							// Sends the ehlo command
							sendCommand(
								ClientCommandType::CCT_EHLO,
								{domain}
							);
							DEBUG_ONLY(this->printSent("EHLO [domain]"));

							// Receives the multiline response and checks for
							// - any transmission errors
							try {
								vector<string> options = {};

								// Reads all the command
								for (;;) {
									// Cuts the code of the line and then puts
									// - the option inside of the vector, after that
									// - we check if we need to do another round
									string line = client.readToDelim("\r\n");

									string command;
									reduceWhitespace(line.substr(4), command);
									removeFirstAndLastWhite(command);
									options.push_back(command);
									
									if (line[3] == '-')
										continue;
									else
										break;
								}

								for (const string &option : options) {
									DEBUG_ONLY(this->printReceived(250, option));

									// Checks which flag we need to set, these will
									// - tell the algorithm what actions to perform at what
									// - moment
									if (option == "STARTTLS")
										session.setFlag(_SCS_FLAG_STARTTLS);
									else if (option == "8BITMIME")
										session.setFlag(_SCS_FLAG_8BITMIME);
									else if (option == "ENHANCEDSTATUSCODES")
										session.setFlag(_SCS_FLAG_ENCHANCHED_STATUS_CODES);
									else if (option == "PIPELINING")
										session.setFlag(_SCS_FLAG_PIPELINGING);
									else if (option == "CHUNKING")
										session.setFlag(_SCS_FLAG_CHUNKING);
								}

								// Disables the parser for one round, and sets
								// - the args and code
								_performParse = false;
								code = 250;
								args = options[0];
							} catch (const runtime_error &e) {
								_run = false;
							}
							continue;
						}

						// Checks if we need to perform the hello command without
						// - the ESMTP manner
						if (
							!session.getAction(_SCS_ACTION_HELO) &&
							!session.getFlag(_SCS_FLAG_ESMTP)
						) {
							session.setAction(_SCS_ACTION_HELO);

							// Sends the helo command
							sendCommand(
								ClientCommandType::CCT_HELO,
								{domain}
							);
							DEBUG_ONLY(this->printSent("HELO [domain]"));
						}
					}
					default: {

						string message = "Transmission error: ";
						message += to_string(code) + ": " + args;
						this->addError(target.address.toString(), message);
						if (!this->s_Silent) this->s_Logger << ERROR << message << ENDL << CLASSIC;

						_run = false;
						continue;
					}
				}
			}
		}

		if (!this->s_Silent) {
			this->s_Logger << "Handled " << this->s_Targets.size() << " emails" << ENDL;
		}

		return *this;
	}

	SMTPClient &SMTPClient::printReceived(const int32_t code, const string &args) {
		this->s_Logger << DEBUG << "S->[code:" << code << "]: " << args << ENDL << CLASSIC;
		return *this;
	}

	SMTPClient &SMTPClient::printSent(const string &mess) {
		this->s_Logger << DEBUG << "C->RESP: " << mess << ENDL << CLASSIC;
		return *this;
	}
}