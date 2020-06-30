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
	ServerResponse::ServerResponse(
		const SMTPResponseCommand &r_CType,
		const std::string &r_Message
	): r_CType(r_CType)
	{
		switch (r_CType)
		{
			case SMTPResponseCommand::SRC_PROCEED:
			{
				this->r_Message = "250";
				break;
			}
			case SMTPResponseCommand::SRC_SYNTAX_ARG_ERR:
			{
				this->r_Message = "501";
				break;
			}
			case SMTPResponseCommand::SRC_SYNTAX_ERR_INVALID_COMMAND:
			{
				this->r_Message += "500";
				break;
			}
			default:
			{
				throw std::runtime_error("ServerResponse::ServerResponse() failed: invalid command type selected !");
				break;
			}
		}

		this->r_Message += ' ';
		this->r_Message += r_Message;
	}

	ServerResponse::ServerResponse(
		const SMTPResponseCommand &r_CType,
		const bool &r_ESMTP,
		const std::vector<SMTPServiceFunction> *services
	):
		r_CType(r_CType), r_ESMTP(r_ESMTP)
	{
		// Because no message is supplied, we assume he / she
		// - is just lazy af and we will supply an default
		// - one ;/
		switch (r_CType)
		{
			case SMTPResponseCommand::SRC_PROCEED:
			{
				this->r_Message = "250 OK";
				break;
			}
			case SMTPResponseCommand::SRC_READY_START_TLS:
			{
				this->r_Message = "250 Go ahead";
				break;
			}
			case SMTPResponseCommand::SRC_SYNTAX_ERR_INVALID_COMMAND:
			{
				this->r_Message = "500 Syntax error: invalid command";
				break;
			}
			case SMTPResponseCommand::SRC_HELO_RESP:
			{
				if (!r_ESMTP)
				{
					this->r_Message += "250";
					this->r_Message = _SMTP_SERVICE_DOMAIN;
					this->r_Message += ", nice to meet you !";
				} else
				{
					// Checks if there are any services, and if
					// - so it will append them, if services is
					// - nullptr get default message
					if (services == nullptr)
					{
						this->r_Message += "250 ";
						this->r_Message += _SMTP_SERVICE_DOMAIN;
						this->r_Message += ", nice to meet you !";
					} else
					{
						this->r_Message += "250-";
						this->r_Message += _SMTP_SERVICE_DOMAIN;
						this->r_Message += ", nice to meet you !\r\n";

						std::size_t i = 0;
						for (const SMTPServiceFunction &s : *services)
						{
							if (++i >= services->size())
							{
								// Appends the default status code and 
								// - the command name, after that the sub args
								this->r_Message += "250 ";
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
								this->r_Message += "250-";
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
				this->r_Message += "220 ";
				this->r_Message = _SMTP_SERVICE_DOMAIN;
				if (r_ESMTP)
					this->r_Message += " ESMTP ";
				else
					this->r_Message += " SMTP ";
				this->r_Message += _SMTP_SERVICE_NODE_NAME;
				break;
			}
			case SMTPResponseCommand::SRC_QUIT_RESP:
			{
				this->r_Message += "221 closing connection ";
				this->r_Message += _SMTP_SERVICE_NODE_NAME;
				break;
			}
			case SMTPResponseCommand::SRC_SYNTAX_ARG_ERR:
			{
				this->r_Message += "501";
				this->r_Message += " Syntax Error";
				break;
			}
		}
	}

	void ServerResponse::build(std::string &ret)
	{
		// Builds the response message,
		// - and adds the server name if
		// - ESMTP enabled
		switch (this->r_CType)
		{
			case SRC_INIT:
			{
				ret += "220 ";
				break;
			}
			case SRC_HELO_RESP:
			{
				break;
			}
		}
		ret += this->r_Message;
		if (this->r_ESMTP && this->r_CType != SRC_HELO_RESP)
			ret += " - fsmtp";
		ret += "\r\n";
	}
}