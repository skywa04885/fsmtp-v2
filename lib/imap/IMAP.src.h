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

#pragma once

#include <string>
#include <vector>
#include <cassert>
#include <cstdint>
#include <atomic>
#include <memory>
#include <functional>
#include <regex>
#include <thread>
#include <cctype>
#include <variant>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../general/Logger.src.h"
#include "../general/macros.src.h"
#include "../general/exceptions.src.h"
#include "../general/cleanup.src.h"
#include "../general/connections.src.h"
#include "../models/Account.src.h"
#include "../models/RawEmail.src.h"
#include "../models/Email.src.h"
#include "../models/LocalDomain.src.h"
#include "../models/EmailShortcut.src.h"
#include "../models/Mailbox.src.h"
#include "../general/Passwords.src.h"
#include "../models/MailboxStatus.src.h"

using namespace FSMTP::Cleanup;
using namespace FSMTP::Models;
using namespace FSMTP::Connections;

class IMAPBad : std::exception
{
public:
	IMAPBad(const std::string &e_Message):
		e_Message(e_Message)
	{}

	const char *what(void) const throw()
	{ return this->e_Message.c_str(); }
private:
	std::string e_Message;
};

class IMAPNo : std::exception
{
public:
	IMAPNo(const std::string &e_Message):
		e_Message(e_Message)
	{}

	const char *what(void) const throw()
	{ return this->e_Message.c_str(); }
private:
	std::string e_Message;
};
