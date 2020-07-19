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

namespace FSMTP::Server
{
	class SyntaxException : public std::exception
	{
	public:
		SyntaxException(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what() const throw()
    {
    	return this->e_Message.c_str();
    }

	private:
		std::string e_Message;
	};

	class CommandOrderException : public std::exception
	{
	public:
		CommandOrderException(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what() const throw()
    {
    	return this->e_Message.c_str();
    }
	private:
		std::string e_Message;
	};

	class FatalException : public std::exception
	{
	public:
		FatalException(const std::string &e_Message):
			e_Message(e_Message)
		{}

		const char *what() const throw()
    {
    	return this->e_Message.c_str();
    }
	private:
		std::string e_Message;
	};
}