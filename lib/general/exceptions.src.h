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
#include <stdexcept>

namespace FSMTP
{
	// =========================================
	// Socket exceptions
	//
	// Exceptions which occur in transport
	// =========================================

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

	class SocketSSLError : std::exception
	{
	public:
		SocketSSLError(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what(void) const throw()
		{ return this->e_Message.c_str(); }
	private:
		std::string e_Message;
	};

	class InvalidCommand : std::exception
	{
	public:
		InvalidCommand(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what(void) const throw()
		{ return this->e_Message.c_str(); }
	private:
		std::string e_Message;
	};

	class SyntaxError : std::exception
	{
	public:
		SyntaxError(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what(void) const throw()
		{ return this->e_Message.c_str(); }
	private:
		std::string e_Message;
	};

	class OrderError : std::exception
	{
	public:
		OrderError(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what(void) const throw()
		{ return this->e_Message.c_str(); }
	private:
		std::string e_Message;
	};

	// =========================================
	// Database exceptions
	//
	// Exceptions which occur in database query
	// -'s etcetera
	// =========================================

	class EmptyQuery : std::exception
	{
	public:
		EmptyQuery(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what(void) const throw()
		{ return this->e_Message.c_str(); }
	private:
		std::string e_Message;
	};

	class DatabaseException : std::exception
	{
	public:
		DatabaseException(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what(void) const throw()
		{ return this->e_Message.c_str(); }
	private:
		std::string e_Message;
	};
}