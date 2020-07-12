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
		s_Logger("", LoggerLevel::INFO)
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
				std::vector<DNS::Record> records = DNS::resolveDNSRecords(domain, 
					DNS::RT_MX); 
				for (DNS::Record &record : records)
				{
					std::string server = DNS::resolveHostname(record.r_Value.c_str());
					target.t_Servers.push_back(server);
					if (!s_Silent) this->s_Logger << "Added target server: " << server << ENDL;
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
			} catch (const std::runtime_error &e)
			{
				this->s_Logger << ERROR << "Could not connect to server: " << e.what() << ENDL;
				this->addError(SMTPClientPhase::SCP_CONNECT, e.what());
				continue;
			}

			// ===============================
			// Starts sending the message
			// 
			// Starts sending the message over
			// - the socket
			// ===============================

			// Infinite read loop
			// while (true)
			// {

			// }
		}
	}
}