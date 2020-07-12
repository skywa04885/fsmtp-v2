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

		// Resolves the hostnames and sets the status if it
		// - could not be found
		

		// Sets the targets, and composes the message
		this->s_Targets = config.m_To;
		this->s_TransportMessage = compose(config);
	}
}