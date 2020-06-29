#include "Response.src.h"

namespace FSMTP::SMTP
{
	ServerResponse::ServerResponse(
		const SMTPResponseCommand &r_CType,
		const bool &r_ESMTP,
		std::vector<SMTPServiceFunction> *services
	):
		r_CType(r_CType), r_ESMTP(r_ESMTP)
	{
		// Because no message is supplied, we assume he / she
		// - is just lazy af and we will supply an default
		// - one ;/
		switch (r_CType)
		{
			case SRC_PROCEED:
			{
				this->r_Message = "OK";
				break;
			}
			case SRC_HELO_RESP:
			{
				if (!r_ESMTP)
				{
					this->r_Message = _SMTP_SERVICE_DOMAIN;
					this->r_Message += ", nice to meet you !";
				} else
				{
					// Checks if there are any services, and if
					// - so it will append them, if services is
					// - nullptr get default message
					if (services == nullptr)
					{
						this->r_Message = _SMTP_SERVICE_DOMAIN;
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
			case SRC_INIT:
			{
				if (r_ESMTP)
				{
					this->r_Message = _SMTP_SERVICE_DOMAIN;
					this->r_Message += " ESMTP ";
					this->r_Message += _SMTP_SERVICE_NODE_NAME;
				} else
				{
					this->r_Message = _SMTP_SERVICE_DOMAIN;
					this->r_Message += " SMTP ";
					this->r_Message += _SMTP_SERVICE_NODE_NAME;
				}
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