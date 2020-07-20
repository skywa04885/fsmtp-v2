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

#include "IMAPServer.src.h"

namespace FSMTP::IMAP
{
	IMAPServer::IMAPServer(const int32_t plainPort, const int32_t securePort):
		s_Socket(plainPort, securePort), s_SecureRunning(false),
		s_PlainRunning(false), s_Run(true)
	{
		// Starts listening
		this->s_Socket.startListening(
			&this->s_PlainRunning,
			&this->s_SecureRunning,
			&this->s_Run,
			&IMAPServer::acceptorCallback,
			this
		);

		// Sets the shared capability's
		this->s_SecureCapabilities.push_back(IMAPCapability{
			"IMAP4rev1", nullptr
		});
		this->s_PlainCapabilities.push_back(IMAPCapability{
			"IMAP4rev1", nullptr
		});

		// Sets the secure only capability's
		this->s_SecureCapabilities.push_back(IMAPCapability{
			"LOGIN", nullptr
		});

		// Sets the plain only capability's
		this->s_PlainCapabilities.push_back(IMAPCapability{
			"STARTTLS", nullptr
		});
		this->s_PlainCapabilities.push_back(IMAPCapability{
			"LOGINDISABLED", nullptr
		});
	}

	void IMAPServer::acceptorCallback(
		std::unique_ptr<IMAPClientSocket> client,
		void *u
	)
	{
		IMAPServer &server = *reinterpret_cast<IMAPServer *>(u);
		std::string prefix = "IMAPAcceptor:";
		prefix += inet_ntoa(client->s_Addr.sin_addr);
		Logger logger(prefix, LoggerLevel::INFO);

		// Sends the initial response
		client->sendResponse(
			IRS_NT, "",
			IMAPResponseType::IRT_GREETING,
			IMAPResponsePrefixType::IPT_OK,
			nullptr
		);

		// Starts the communication loop
		for (;;)
		{
			IMAPCommand command;

			try {
				std::string raw = client->readUntilCRLF();
				command.parse(raw);
			} catch (const SocketReadException &e)
			{
				logger << FATAL << "Could not read data" << ENDL << CLASSIC;
				break;
			} catch (const std::length_error &e)
			{
				logger << FATAL << "Length error (most likely CMD sucker): " << e.what() << ENDL << CLASSIC;
				break;
			}

			// Checks how to respond to the command
			try
			{
				switch (command.c_Type)
				{
					// ===============================================
					// Handles the 'CAPABILITY' command
					//
					// Shows what the server is capable of
					// ===============================================
					case IMAPCommandType::ICT_CAPABILITY:
					{
						// Checks if we are in plain or secure mode
						// - this effects the kind of commands we may use
						if (client->s_UseSSL)
						{
							client->sendResponse(
								IRS_NTATL, command.c_Index,
								IMAPResponseType::IRT_CAPABILITIES,
								IMAPResponsePrefixType::IPT_OK,
								&server.s_SecureCapabilities
							);
						} else
						{
							client->sendResponse(
								IRS_NTATL, command.c_Index,
								IMAPResponseType::IRT_CAPABILITIES,
								IMAPResponsePrefixType::IPT_OK,
								&server.s_PlainCapabilities
							);
						}
						break;
					}
					// ===============================================
					// Handles the 'LOGIN' command
					//
					// Authenticates the user, responses:
					// - OK: Login completed, now in authenticated
					// state
					// - NO: Login failure: user or password rejected
					// - BAD: Command unknown / arguments invalid
					// ===============================================
					case IMAPCommandType::ICT_LOGIN:
					{
						// Tries to login, in the mean time handles the
						// - errors, and sends them
						try
						{

						} catch (const IMAPBad& e)
						{

						} catch (const IMAPNo& e)
						{
							client->sendResponse(
								IRS_TL, command.c_Index,
								IMAPResponseType::IRT_LOGOUT,
								IMAPResponsePrefixType::IPT_OK,
								nullptr
							);
						}
						break;
					}
					// ===============================================
					// Handles the 'LOGOUT' command
					//
					// Closes the session, and shuts the connection
					// - down
					// ===============================================
					case IMAPCommandType::ICT_LOGOUT:
					{
						// Sends the message and terminates
						client->sendResponse(
							IRS_NTATL, command.c_Index,
							IMAPResponseType::IRT_LOGOUT,
							IMAPResponsePrefixType::IPT_OK,
							nullptr
						);
						goto _imap_server_acceptor_end;
					}
					// ===============================================
					// Handles the unknown commands
					//
					// Throws unknown command error
					// ===============================================
					case IMAPCommandType::ICT_UNKNOWN:
					default:
					{
						throw InvalidCommand("Command not valid");
					}
				}
			} catch (const InvalidCommand &e)
			{

			}
		}

	_imap_server_acceptor_end: return;
	}
}