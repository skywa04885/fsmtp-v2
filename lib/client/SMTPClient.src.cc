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
		s_Logger("SMTPClient", LoggerLevel::INFO)
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
				this->addError(SMTPClientPhase::SCP_RESOLVE, e.what());	
			}
		}
	}

	/**
	 * Adds an error to the error log
	 *
	 * @Param {const SMTPClientPhase} phase
	 * @Param {const std::string &} message
	 * @Return {void}
	 */
	void SMTPClient::addError(
		const SMTPClientPhase phase,
		const std::string &message
	)
	{
		this->s_ErrorLog.push_back(SMTPClientError{
			phase,
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch()
			).count(),
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
				this->addError(SMTPClientPhase::SCP_CONNECT, e.what());
				continue;
			}

			// ===============================
			// Starts sending the message
			// 
			// Starts sending the message over
			// - the socket
			// ===============================

			SMTPClientSession session;

			// Infinite read loop
			bool _run = true;
			while (_run)
			{
				std::string responseArgs;
				int32_t responseCode;

				// ================================
				// Performs StartTLS
				//
				// If this is so, we need to send
				// - helo again
				// ================================

				// Since STARTTLS may require an early hello, we will
				// - always check if this needs to be performed
				if (
					session.getAction(_SMTP_CLIENT_SESSION_ACTION_START_TLS_FINISHED) &&
					!session.getAction(_SMTP_CLIENT_SESSION_ACTION_START_TLS_HELO)
				)
				{
					session.setAction(_SMTP_CLIENT_SESSION_ACTION_START_TLS_HELO);

					// Builds the arg, and builds the command
					std::string arg = "[";
					arg += _SMTP_SERVICE_DOMAIN;
					arg += ']';
					
					ClientCommand command(ClientCommandType::CCT_EHLO, {
						arg
					});
					std::string response = command.build();

					// Sends the command to the client
					client->sendMessage(response);
					DEBUG_ONLY(this->printSent(response));
				}


				// ================================
				// Parses the response
				//
				// Lets us know what the server
				// - responded
				// ================================


				// Receives the string from the client, and if debug enables
				// - we print it to the console, next to that we will also
				// - parse the command into an code and argument
				try
				{
					std::string buffer = client->receive();
					std::tie(responseCode, responseArgs) = ServerResponse::parseResponse(buffer);
					DEBUG_ONLY(this->printReceived(responseCode, responseArgs));
				} catch (const SMTPTransmissionError &e)
				{
					this->s_Logger << "SMTPClientSocket::receive() failed: " << e.what() << ENDL;
					_run = false;
					continue;
				}

				// ================================
				// Performs operation
				//
				// Checks the code, and current
				// - actions and decides what to
				// - do next
				// ================================

				switch (responseCode)
				{
					case 220:
					{
						if (!session.getAction(_SMTP_CLIENT_SESSION_ACTION_HELO))
						{
							session.setAction(_SMTP_CLIENT_SESSION_ACTION_HELO);

							// Builds the arg, and builds the command
							ClientCommand command(ClientCommandType::CCT_EHLO, {
								_SMTP_SERVICE_DOMAIN
							});
							std::string response = command.build();

							// Checks if we're using ESMTP, and sets the flag
							// - if so
							if (responseArgs.find("ESMTP") != std::string::npos)
							{
								DEBUG_ONLY(this->s_Logger << DEBUG << "Server meldt zichzelf als " 
									"ESMTP, inschakelen extra functies .." << ENDL << CLASSIC);
								session.setFlag(_SMTP_CLIENT_SESSION_FLAG_ESMTP);
							}

							// Sends the command to the client
							client->sendMessage(response);
							DEBUG_ONLY(this->printSent(response));
							continue;
						}

						if (
							session.getAction(_SMTP_CLIENT_SESSION_ACTION_START_TLS) &&
							!session.getAction(_SMTP_CLIENT_SESSION_ACTION_START_TLS_FINISHED)
						)
						{
							session.setAction(_SMTP_CLIENT_SESSION_ACTION_START_TLS_FINISHED);
							DEBUG_ONLY(
								this->s_Logger << DEBUG 
									<< "Aanvraag geacepteerd, verbind wordt beveiligd !" 
									<< ENDL << CLASSIC;
							);
							client->upgradeToSSL();
						}

						break;
					}			
					case 250:
					{
						// Checks if we may switch to an TLS socket, this may only happen
						// - if the server is an ESMTP server
						if (
							!session.getAction(_SMTP_CLIENT_SESSION_ACTION_START_TLS) &&
							session.getFlag(_SMTP_CLIENT_SESSION_FLAG_ESMTP)
						)
						{
							DEBUG_ONLY(
								this->s_Logger << DEBUG << "ESMTP is actief, beginnen met aanvraag "
								"van veilige verbinding ..." << ENDL << CLASSIC;
							);

							session.setAction(_SMTP_CLIENT_SESSION_ACTION_START_TLS);
							ClientCommand command(ClientCommandType::CCT_START_TLS, {});
							std::string response = command.build();

							client->sendMessage(response);
							DEBUG_ONLY(this->printSent(response));
							continue;
						}

						if (!session.getAction(_SMTP_CLIENT_SESSION_ACTION_MAIL_FROM))
						{
							session.setAction(_SMTP_CLIENT_SESSION_ACTION_MAIL_FROM);
							ClientCommand command(ClientCommandType::CCT_MAIL_FROM, {
								"<" + this->s_MailFrom.e_Address + ">"
							});
							std::string response = command.build();

							client->sendMessage(response);
							DEBUG_ONLY(this->printSent(response));
							continue;
						}

						if (!session.getAction(_SMTP_CLIENT_SESSION_ACTION_RCPT_TO))
						{

							session.setAction(_SMTP_CLIENT_SESSION_ACTION_RCPT_TO);
							ClientCommand command(ClientCommandType::CCT_RCPT_TO, {
								"<" + target.t_Address.e_Address + ">"
							});
							std::string response = command.build();

							client->sendMessage(response);
							DEBUG_ONLY(this->printSent(response));
							continue;
						}

						if (!session.getAction(_SMTP_CLIENT_SESSION_ACTION_DATA_START))
						{
							session.setAction(_SMTP_CLIENT_SESSION_ACTION_DATA_START);
							ClientCommand command(ClientCommandType::CCT_DATA, {});
							std::string response = command.build();

							client->sendMessage(response);
							DEBUG_ONLY(this->printSent(response));
							continue;
						}

						if (
							session.getAction(_SMTP_CLIENT_SESSION_ACTION_DATA_BUSSY) &&
							!session.getAction(_SMTP_CLIENT_SESSION_ACTION_DATA_END)
						)
						{
							session.setAction(_SMTP_CLIENT_SESSION_ACTION_DATA_END);

							ClientCommand command(ClientCommandType::CCT_QUIT, {});
							std::string response = command.build();

							client->sendMessage(response);
							DEBUG_ONLY(this->printSent(response));

							_run = false;
							continue;
						}

						break;
					}

					case 354:
					{
						if (
							session.getAction(_SMTP_CLIENT_SESSION_ACTION_DATA_START) &&
							!session.getAction(_SMTP_CLIENT_SESSION_ACTION_DATA_BUSSY)
						)
						{
							session.setAction(_SMTP_CLIENT_SESSION_ACTION_DATA_BUSSY);

							client->sendMessage(this->s_TransportMessage);
							DEBUG_ONLY(this->printSent("[-> Body <-]\r\n"));
							client->sendMessage("\r\n.\r\n");
							continue;
						}
					}

					default:
					{
						this->addError(
							SMTPClientPhase::SCP_OTHER, 
							std::to_string(responseCode) + ": " + responseArgs
						);
					}
				}
			}
		}

		if (!this->s_Silent) this->s_Logger << "Handles" << this->s_Targets.size() << " emails" << ENDL;
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
		this->s_Logger << DEBUG << "C->RESP: " << mess.substr(0, mess.size() - 2) << ENDL << CLASSIC;
	}
}