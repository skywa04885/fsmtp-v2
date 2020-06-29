#include <vector>
#include <string>
#include <cstdint>

#pragma once

#ifndef _SMTP_SERVICE_DOMAIN
#define _SMTP_SERVICE_DOMAIN "smtp.fannst.nl"
#endif

#ifndef _SMTP_SERVICE_NODE_NAME
#define _SMTP_SERVICE_NODE_NAME "fclust-node01"
#endif

namespace FSMTP::SMTP
{
	typedef enum : uint32_t {
		SRC_INIT = 0,
		SRC_HELO_RESP,
		SRC_READY_START_TLS,
		SRC_PROCEED
	} SMTPResponseCommand;

	typedef struct {
		const char *s_Name;
		const std::vector<const char *> s_SubArgs;
	} SMTPServiceFunction;

	class ServerResponse
	{
	public:
		ServerResponse(
			const SMTPResponseCommand &r_CType,
			const bool &r_ESMTP, 
			std::vector<SMTPServiceFunction> *services
		);
		ServerResponse(const SMTPResponseCommand &r_CType, const std::string &r_Message);

		void build(std::string &ret);
	private:
		std::string r_Message;
		SMTPResponseCommand r_CType;
		bool r_ESMTP;
	};
}