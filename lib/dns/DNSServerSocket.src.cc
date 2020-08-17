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

#include "DNSServerSocket.src.h"

namespace FSMTP::DNS
{
	DNSServerSocket::DNSServerSocket(const int32_t port):
		s_Logger("DNSServerSock", LoggerLevel::INFO)
	{
		int32_t rc;

		// Creates the server socket address
		memset(&this->s_SocketAddr, 0x0, sizeof (struct sockaddr_in));
		this->s_SocketAddr.sin_addr.s_addr = INADDR_ANY;
		this->s_SocketAddr.sin_family = AF_INET;
		this->s_SocketAddr.sin_port = htons(port);

		// Creates the socket and checks for errors
		this->s_SocketFD = socket(AF_INET, SOCK_DGRAM, 0x0);
		if (this->s_SocketFD < 0)
		{
			std::string error = "socket() failed: ";
			error += strerror(errno);
			throw runtime_error(error);
		}

		// Sets the socket to reuse the old address
		int32_t opt = 0x1;
		rc = setsockopt(this->s_SocketFD, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&opt), sizeof (opt));
		if (rc < 0)
		{
			std::string error = "setsockopt() failed: ";
			error += strerror(rc);
			throw std::runtime_error(error);
		}

		// Binds the socket
		rc = bind(
			this->s_SocketFD, 
			reinterpret_cast<struct sockaddr *>(&this->s_SocketAddr), 
			sizeof (struct sockaddr)
		);
		if (rc < 0)
		{
			std::string error = "bind() failed: ";
			error += strerror(errno);
			throw runtime_error(error);
		}
	}

	void DNSServerSocket::startAcceptorSync(
		std::atomic<bool> *run,
		std::atomic<bool> *running,
		const std::function<void(DNSServerSocket *, void *)> cb,
		void *u
	)
	{
		// Creates the thread and detaches
		std::thread t(&DNSServerSocket::syncAcceptorThread, this, 
			run, running, cb, u);
		t.detach();
	}

	void DNSServerSocket::syncAcceptorThread(
		std::atomic<bool> *run,
		std::atomic<bool> *running,
		const std::function<void(DNSServerSocket *, void *)> cb,
		void *u
	)
	{
		// Sets running to true, and loops while we're saying
		// - it should
		*running = true;
		while (*run == true)
		{
			cb(this, u);
		}

		// Sets running to false, and gives a short delay
		*running = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
	}
}