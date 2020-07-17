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
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <atomic>
#include <algorithm>
#include <tuple>
#include <sstream>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>

#include "../general/Logger.src.h"
#include "../general/macros.src.h"
#include "../general/cleanup.src.h"

#ifndef _POP3_QUEUE_MAX
#define _POP3_QUEUE_MAX 40
#endif

#ifndef _POP3_PORT_PLAIN
#define _POP3_PORT_PLAIN 110
#endif

#ifndef _POP3_PORT_SECURE
#define _POP3_PORT_SECURE 995
#endif

using namespace FSMTP::Cleanup;

namespace FSMTP::POP3
{
	class SocketWriteException : std::exception
	{
	public:
		SocketWriteException(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what(void) const throw()
		{ return this->e_Message.c_str(); }
	private:
		std::string e_Message;
	};

	class SocketReadException : std::exception
	{
	public:
		SocketReadException(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what(void) const throw()
		{ return this->e_Message.c_str(); }
	private:
		std::string e_Message;
	};

	class SocketInitializationException : std::exception
	{
	public:
		SocketInitializationException(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what(void) const throw()
		{ return this->e_Message.c_str(); }
	private:
		std::string e_Message;
	};
}
