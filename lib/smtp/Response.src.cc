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

#include "Response.src.h"

namespace FSMTP::SMTP
{
	/**
	 * The default constructor for an custom user response
	 * - but we will still add the status code by default
	 *
	 * @Param {SMTPResponseCommand &} r_CType
	 * @Param {std::string &} r_Message
	 * @Return void
	 */
	ServerResponse::ServerResponse(
		const SMTPResponseCommand &r_CType,
		const std::string &r_Message
	): r_CType(r_CType)
	{
		// Appends the command code and later the
		// - custom user message
		this->r_Message += ServerResponse::rcToCode(r_CType);
		this->r_Message += ' ';
		this->r_Message += r_Message;
	}

	/**
	 * Default constructor which will throw default responses
	 *
	 * @Param {SMTPResponseCommand &} r_CType
	 * @Param {bool &} r_ESMTP
	 * @Param {std::vector<SMTPServiceFunction> *} services
	 * @Return void
	 */
	ServerResponse::ServerResponse(
		const SMTPResponseCommand &r_CType,
		const bool &r_ESMTP,
		const std::vector<SMTPServiceFunction> *services
	):
		r_CType(r_CType), r_ESMTP(r_ESMTP)
	{
		// Appends the command code and later
		// - the custom prepared messages
		this->r_Message += ServerResponse::rcToCode(r_CType);

		// Because no message is supplied, we assume he / she
		// - is just lazy af and we will supply an default
		// - one ;/
		switch (r_CType)
		{
			case SMTPResponseCommand::SRC_PROCEED:
			{
				this->r_Message += " OK";
				break;
			}
			case SMTPResponseCommand::SRC_READY_START_TLS:
			{
				this->r_Message += " Go ahead";
				break;
			}
			case SMTPResponseCommand::SRC_SYNTAX_ERR_INVALID_COMMAND:
			{
				this->r_Message += " Syntax error: invalid command";
				break;
			}
			case SMTPResponseCommand::SRC_BAD_EMAIL_ADDRESS:
			{
				this->r_Message += " Bad email address";
				break;
			}
			case SMTPResponseCommand::SRC_DATA_START:
			{
				this->r_Message += " Proceed with body, end data with <CR><LF>.<CR><LF>";
				break;
			}
			case SMTPResponseCommand::SRC_DATA_END:
			{
				this->r_Message += " Mail queued for delivery";
				break;
			}
			case SMTPResponseCommand::SRC_HELO_RESP:
			{
				if (!r_ESMTP)
				{
					this->r_Message += ' ';
					this->r_Message += _SMTP_SERVICE_DOMAIN;
					this->r_Message += ", nice to meet you !";
				} else
				{
					// Checks if there are any services, and if
					// - so it will append them, if services is
					// - nullptr get default message
					if (services == nullptr)
					{
						this->r_Message += ' ';
						this->r_Message += _SMTP_SERVICE_DOMAIN;
						this->r_Message += ", nice to meet you !";
					} else
					{
						this->r_Message += '-';
						this->r_Message += _SMTP_SERVICE_DOMAIN;
						this->r_Message += ", nice to meet you !\r\n";

						std::size_t i = 0;
						for (const SMTPServiceFunction &s : *services)
						{
							if (++i >= services->size())
							{
								// Appends the default status code and 
								// - the command name, after that the sub args
								this->r_Message += ServerResponse::rcToCode(r_CType);
								this->r_Message += ' ';
								this->r_Message += s.s_Name;
								
								for (const char *c : s.s_SubArgs)
								{
									this->r_Message += " ";
									this->r_Message += c;
								}

							} else
							{
								// Appends the message and then checks
								// - if there are any sub args or something
								this->r_Message += ServerResponse::rcToCode(r_CType);
								this->r_Message += "-";
								this->r_Message += s.s_Name;

								for (const char *c : s.s_SubArgs)
								{
									this->r_Message += " ";
									this->r_Message += c;
								}

								this->r_Message += "\r\n";							
							}
						}
					}
				}
				break;
			}
			case SMTPResponseCommand::SRC_INIT:
			{
				this->r_Message += ' ';
				this->r_Message += _SMTP_SERVICE_DOMAIN;
				if (r_ESMTP)
					this->r_Message += " ESMTP ";
				else
					this->r_Message += " SMTP ";
				this->r_Message += _SMTP_SERVICE_NODE_NAME;
				break;
			}
			case SMTPResponseCommand::SRC_QUIT_RESP:
			{
				this->r_Message += " closing connection ";
				this->r_Message += _SMTP_SERVICE_NODE_NAME;
				break;
			}
			case SMTPResponseCommand::SRC_SYNTAX_ARG_ERR:
			{
				this->r_Message += " Syntax Error";
				break;
			}
		}
	}

	/**
	 * Builds the response and stores it in the string reference
	 *
	 * @Param {std::string &} ret
	 * @Return void
	 */
	void ServerResponse::build(std::string &ret)
	{
		// Builds the response message,
		// - and adds the server name if
		// - ESMTP enabled
		ret += this->r_Message;
		if (this->r_ESMTP && this->r_CType != SRC_HELO_RESP)
			ret += " - fsmtp";
		ret += "\r\n";
	}
	/**

	 * Static method which turns an enum value
	 * - into an usable code in the string format
	 *
	 * @Param {SMTPResponseCommand &} c
	 * @Return const char *
	 */
	const char *ServerResponse::rcToCode(const SMTPResponseCommand &c)
	{
		switch (c)
		{
			case SMTPResponseCommand::SRC_INIT: return "220";
			case SMTPResponseCommand::SRC_HELO_RESP: return "250";
			case SMTPResponseCommand::SRC_READY_START_TLS: return "250";
			case SMTPResponseCommand::SRC_PROCEED: return "250";
			case SMTPResponseCommand::SRC_QUIT_RESP: return "221";
			case SMTPResponseCommand::SRC_SYNTAX_ERR_INVALID_COMMAND: return "501";
			case SMTPResponseCommand::SRC_SYNTAX_ARG_ERR: return "501";
			case SMTPResponseCommand::SRC_BAD_EMAIL_ADDRESS: return "510";
			case SMTPResponseCommand::SRC_DATA_START: return "354";
			case SMTPResponseCommand::SRC_DATA_END: return "250";
			default: throw std::runtime_error("The programmer messed up, and used an not implemented enum value ..");
		}
	}

	/**
	 * Parses an server response into an string and code
	 *
	 * @Param {const std::string &} raw
	 * @Return {int32_t}
	 * @Return {std::string}
	 */
	std::tuple<int32_t, std::string> ServerResponse::parseResponse(const std::string &raw)
	{
		std::string clean;
		reduceWhitespace(raw, clean);

		std::size_t index = clean.find_first_of(' ');
		if (index == std::string::npos)
			return std::make_pair(std::stoi(clean), "");
		else return std::make_pair(
			std::stoi(clean.substr(0, index)),
			clean.substr(++index)
		);
	}
}