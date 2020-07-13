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

namespace FSMTP::Mailer::Client
{
	/**
	 * Default constructor for the SMTPClient
	 *
	 * @Param {bool} s_Silent
	 * @Return {void}
	 */
	SMTPClient::SMTPClient(bool s_Silent):
		s_Logger("SMTPClient", LoggerLevel::INFO), s_ErrorCount(0)
	{
		if (!s_Silent) this->s_Logger << "SMTPClient initialized !" << ENDL;
	}

	/**
	 * Composes the email message, and sets some options
	 * - inside of the class, such as the message and targets
	 * 
	 * @Param {MailComposerConfig &config}
	 */
	void SMTPClient::prepare(MailComposerConfig &config)
	{
		if (!s_Silent) this->s_Logger << "Preparing ..." << ENDL;

		// Sets the targets, and composes the message
		this->s_TransportMessage = compose(config);
		this->s_MailFrom = config.m_From[0];

		// Resolves the hostnames and sets the status if it
		// - could not be found
		for (EmailAddress &address : config.m_To)
		{
			std::string domain;
			address.getDomain(domain);

			try
			{
				// Creates the target, resolves the addresses and pushes them to the
				// - result vector
				SMTPClientTarget target;
				target.t_Address = address;
				std::vector<DNS::Record> records = DNS::resolveDNSRecords(domain, 
					DNS::RT_MX); 
				
				for (DNS::Record &record : records)
				{
					std::string server = DNS::resolveHostname(record.r_Value.c_str());
					target.t_Servers.push_back(server);
					if (!s_Silent) this->s_Logger << "Server toegevoegd: " << server << ENDL;
				}
				
				this->s_Targets.push_back(target);
			} catch(const std::runtime_error &e)
			{
				std::string message = "Resolver error: ";
				message += e.what();
				this->addError(address.toString(), message);	
			}
		}
	}

	/**
	 * Adds an error to the error log
	 *
	 * @Param {const std::string &} address
	 * @Param {const std::string &} message
	 * @Return {void}
	 */
	void SMTPClient::addError(
		const std::string &address,
		const std::string &message
	)
	{
		s_ErrorCount++;
		this->s_ErrorLog.push_back(SMTPClientError{
			address,
			message
		});
	}

	/**
	 * ( May take a while )
	 * Orders the computer to talk with the other
	 * - servers and transmit the message
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void SMTPClient::beSocial(void)
	{
		if (!this->s_Silent) this->s_Logger << "Versturen naar "
			<< this->s_Targets.size() << " clients .." << ENDL;

		// Loops over the targets and creates the transmission
		// - sockets
		for (SMTPClientTarget &target : this->s_Targets)
		{
			// ===============================
			// Connects to the server
			// 
			// Uses the addresses to connect
			// - to the server
			// ===============================

			// Creates the client SMTPClientSocket, and connects the socket
			std::unique_ptr<SMTPClientSocket> client;
			try
			{
				// Connects to the client and prints it to the console
				client = std::make_unique<SMTPClientSocket>(
					target.t_Servers[0].c_str(),
					25
				);
				client->startConnecting();
				if (!this->s_Silent) this->s_Logger << "Connected to: " 
					<< target.t_Servers[0] << ":25" << ENDL;
			} catch (const SMTPConnectError &e)
			{
				this->s_Logger << ERROR << "Kon niet verbinden met server: " << e.what() << ENDL;
				
				std::string message = "Connection error: ";
				message += e.what();
				this->addError(target.t_Address.toString(), message);	
				continue;
			}

			// ===============================
			// Starts sending the message
			// 
			// Starts sending the message over
			// - the socket
			// ===============================

			SMTPClientSession session;

			bool _run = true;
			bool _performParse = true;


			int32_t code;
			std::string args;

			while (_run)
			{
				// ===================================
				// Receives the default command
				//
				// Receives the data from the server
				// - and stores the code and args
				// ===================================

				if (_performParse)
				{
					try
					{
						std::string line = client->readUntillNewline();
						std::tie(code, args) = ServerResponse::parseResponse(line);
						this->printReceived(code, args);
					} catch(const SMTPTransmissionError &e)
					{
						_run = false;
					} catch (const std::invalid_argument &e)
					{
						this->s_Logger << ERROR << "Invalid response received" << ENDL << CLASSIC;
						_run = false;
					}
				} else _performParse = true;

				// ===================================
				// Acts based on the code
				//
				// Checks the code and acts based
				// - upon it
				// ===================================

				switch (code)
				{
					case 221:
					{
						if (session.getAction(_SCS_ACTION_QUIT))
						{
							_run = false;
							DEBUG_ONLY(this->s_Logger << DEBUG << "Closing transmission channel ..." << ENDL << CLASSIC);
							continue;
						}
					}

					case 250:
					{
						// Checks if we need to perform start tls, this
						// - is only possible if hello is send and esmtp is
						// - enabled
						if (
							session.getAction(_SCS_ACTION_HELO) &&
							!session.getAction(_SCS_ACTION_START_TLS) &&
							session.getFlag(_SCS_FLAG_ESMTP)
						)
						{
							session.setAction(_SCS_ACTION_START_TLS);

							// Sends the STARTTLS command
							client->writeCommand(ClientCommandType::CCT_START_TLS, {});
							this->printSent("STARTTLS");
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
						)
						{
							session.setAction(_SCS_ACTION_MAIL_FROM);

							client->writeCommand(ClientCommandType::CCT_MAIL_FROM, {
								"<" + this->s_MailFrom.e_Address + ">"
							});
							this->printSent("MAIL FROM: [sender]");
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
						)
						{
							session.setAction(_SCS_ACTION_RCPT_TO);

							client->writeCommand(ClientCommandType::CCT_RCPT_TO, {
								"<" + target.t_Address.e_Address + ">"
							});
							this->printSent("RCPT TO: [target]");
							break;
						}

						// Checks if we need to send the data segment, this is only
						// - possible if the rcpt to is already sent
						if (
							session.getAction(_SCS_ACTION_RCPT_TO) &&
							!session.getAction(_SCS_ACTION_DATA_START) &&
							!session.getAction(_SCS_ACTION_DATA_END)
						)
						{
							session.setAction(_SCS_ACTION_DATA_START);

							// Sends the command
							client->writeCommand(ClientCommandType::CCT_DATA, {});
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
						)
						{
							session.setAction(_SCS_ACTION_DATA_END);

							// Writes the body and the "\r\n.\r\n" after that
							client->sendMessage(this->s_TransportMessage);
							client->sendMessage("\r\n.\r\n");
							continue;
						}
					}

					case 220:
					{
						// Checks if we need to send quit, this is only possible if the
						// - message body was sent
						if (
							session.getAction(_SCS_ACTION_DATA_END) &&
							!session.getAction(_SCS_ACTION_QUIT)
						)
						{
							session.setAction(_SCS_ACTION_QUIT);

							// Writes the command
							client->writeCommand(ClientCommandType::CCT_QUIT, {});
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
						)
						{
							session.setAction(_SCS_ACTION_START_TLS_FINISHED);

							// Upgrades the socket
							DEBUG_ONLY(this->s_Logger << DEBUG << "Omzetten naar TLS verbinding ..." << ENDL << CLASSIC);
							try
							{
								client->upgradeToSSL();
								DEBUG_ONLY(this->s_Logger << DEBUG << "Verbinding omgezet !" << ENDL << CLASSIC);								
							} catch (const SMTPSSLError &e)
							{
								std::string message = "SSL Initialization error: ";
								message += e.what();
								this->addError(target.t_Address.toString(), message);
								
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
						)
						{
							if (args.find("ESMTP") != std::string::npos)
								session.setFlag(_SCS_FLAG_ESMTP);
						}

						// Checks if we need to send the initial hello message
						// - using the ESMTP protocol
						if (
							!session.getAction(_SCS_ACTION_HELO) &&
							session.getFlag(_SCS_FLAG_ESMTP)
						)
						{ // -> Initial HELO command
							session.setAction(_SCS_ACTION_HELO);

							// Sends the ehlo command
							client->writeCommand(
								ClientCommandType::CCT_EHLO,
								{_SMTP_SERVICE_DOMAIN}
							);
							this->printSent("EHLO [domain]");

							// Receives the multiline response and checks for
							// - any transmission errors
							try
							{
								std::vector<std::string> options = {};

								// Reads all the command
								for (;;)
								{
									// Cuts the code of the line and then puts
									// - the option inside of the vector, after that
									// - we check if we need to do another round
									std::string line = client->readUntillNewline();
									
									std::string command;
									reduceWhitespace(line.substr(4), command);
									removeFirstAndLastWhite(command);
									options.push_back(command);
									
									if (line[3] == '-')
										continue;
									else
										break;
								}

								for (const std::string &option : options)
								{
									this->printReceived(250, option);

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
							} catch (const SMTPTransmissionError &e)
							{
								_run = false;
							}
							continue;
						}

						// Checks if we need to perform the hello command without
						// - the ESMTP manner
						if (
							!session.getAction(_SCS_ACTION_HELO) &&
							!session.getFlag(_SCS_FLAG_ESMTP)
						)
						{
							session.setAction(_SCS_ACTION_HELO);

							// Sends the helo command
							client->writeCommand(
								ClientCommandType::CCT_HELO,
								{_SMTP_SERVICE_DOMAIN}
							);
							this->printSent("HELO [domain]");
						}
					}
					default:
					{
						std::string message = "Transmission error: ";
						message += std::to_string(code) + ": " + args;
						this->addError(target.t_Address.toString(), message);

						_run = false;
						continue;
					}
				}
			}
		}

		if (!this->s_Silent) this->s_Logger << "Handled " << this->s_Targets.size() << " emails" << ENDL;
	}

	/**
	 * Prints something we recieved from the client
	 *
	 * @Param {const int32_t} code
	 * @Param {const std::string &} args\
	 * @Return {void}
	 */
	void SMTPClient::printReceived(const int32_t code, const std::string &args)
	{
		this->s_Logger << DEBUG << "S->[code:" 
			<< code << "]: " << args << ENDL << CLASSIC;
	}

	/**
	 * Prints something we sent to the console
	 *
	 * @Param {const std::string &} mess
	 * @Return {void}
	 */
	void SMTPClient::printSent(const std::string &mess)
	{
		this->s_Logger << DEBUG << "C->RESP: " << mess << ENDL << CLASSIC;
	}
}