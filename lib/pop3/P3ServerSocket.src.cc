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

#include "P3ServerSocket.src.h"

namespace FSMTP::POP3
{
	ServerSocket::ServerSocket(int32_t port):
		s_Logger("P3ServSock", LoggerLevel::INFO)
	{
		Logger &logger = this->s_Logger;
		int32_t rc;

		// Initialises the socket address structure
		memset(&this->s_SocketAddr, 0, sizeof(struct sockaddr_in));
		this->s_SocketAddr.sin_family = AF_INET;										// IPv4
		this->s_SocketAddr.sin_port = htons(port);			// 995
		this->s_SocketAddr.sin_addr.s_addr = INADDR_ANY;						// All allowed

		// Creates the socket
		this->s_SocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (this->s_SocketFD < 0)
		{
			std::string error = "socket() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		// Sets the socket reuse address property
		int32_t option = 0x1;
		rc = setsockopt(
			this->s_SocketFD,
			SOL_SOCKET,
			SO_REUSEADDR,
			reinterpret_cast<char *>(&option),
			sizeof(option)
		);
		if (rc < 0)
		{
			std::string error = "setsockopt() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		// Makes the socket non blocking
		option = 0x1;
		rc = ioctl(this->s_SocketFD, FIONBIO, reinterpret_cast<char *>(&option));
		if (rc < 0)
		{
			std::string error = "ioctl() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(EXCEPT_DEBUG(error));
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
			throw SocketInitializationException(EXCEPT_DEBUG(error));
		}

		logger << "Initialized on port: " << port << ENDL;
	}

	void ServerSocket::startListening(
		std::atomic<bool> *running,
		std::atomic<bool> *run,
		const std::function<void(std::unique_ptr<ClientSocket>, void *)> callback,
		void *u
	)
	{
		int32_t rc;

		// Listens the socket
		rc = listen(this->s_SocketFD, _POP3_QUEUE_MAX);
		if (rc < 0)
		{
			std::string error = "listen() failed: ";
			error += strerror(errno);
			throw SocketInitializationException(error);
		}

		// Starts the accepting thread
		std::thread acceptorThread(
			&ServerSocket::acceptingThread,
			this,
			running,
			run,
			callback,
			u
		);
		acceptorThread.detach();
	}

	void ServerSocket::acceptingThread(
		std::atomic<bool> *running,
		std::atomic<bool> *run,
		const std::function<void(std::unique_ptr<ClientSocket>, void *)> callback,
		void *u
	)
	{
		int32_t fd, rc;
		struct sockaddr_in client;
		int32_t clientSize = sizeof (struct sockaddr);
		Logger &logger = this->s_Logger;

		logger << "Accepting thread started, waiting for clients !" << ENDL;
		*running = true;
		while (*run == true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			// Tries to accept a new client
			fd = accept(
				this->s_SocketFD,
				reinterpret_cast<struct sockaddr *>(&client),
				reinterpret_cast<socklen_t *>(&clientSize)
			);
			if (fd < 0)
			{
				if (errno != EWOULDBLOCK)
					logger << ERROR << "accept() failed: " << strerror(errno) << ENDL << CLASSIC;
				continue;
			}

			// Sets the socket timeout
			struct timeval tv;
			tv.tv_usec = 0;
			tv.tv_sec = 15;

			rc = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&tv), sizeof(struct timeval));
			if (rc < 0)
			{
				logger << ERROR << "setsockopt() failed: " << strerror(errno) << ENDL << CLASSIC;
				continue;
			}

			rc = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&tv), sizeof(struct timeval));
			if (rc < 0)
			{
				logger << ERROR << "setsockopt() failed: " << strerror(errno) << ENDL << CLASSIC;
				continue;
			}

			// Creates the new thread
			std::thread callbackThread(callback, std::make_unique<ClientSocket>(fd, client), u);
			callbackThread.detach();
		}
		*running = false;
		logger << "Accepting thread closed" << ENDL;
	}

	/**
	 * Stops the server
	 *
	 * @Param {std::atomic<bool> *} running
	 * @Param {std::atomic<bool> *} run
	 * @Return {void}
	 */
	void ServerSocket::shutdown(std::atomic<bool> *running, std::atomic<bool> *run)
	{
		*run = false;
		while (*running == true) continue;
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
	}
}
