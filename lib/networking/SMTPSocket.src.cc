#include "SMTPSocket.src.h"

namespace FSMTP::Networking
{
	SMTPSocket::SMTPSocket(
		const SMTPSocketType &s_SocketType,
		const int32_t &s_SocketPort
	): s_SocketType(s_SocketType), s_SocketPort(s_SocketPort)
	{
		if (s_SocketType == SMTPSocketType::SST_CLIENT)
		{

		} else if (s_SocketType == SMTPSocketType::SST_SERVER)
		{
			int32_t rc;

			// Zeros the memory inside the socket address
			// - after that we configure the socket stuff
			memset(&this->s_SockAddr, 0x0, sizeof (sockaddr_in));
			this->s_SockAddr.sin_addr.s_addr = INADDR_ANY;
			this->s_SockAddr.sin_family = AF_INET;
			this->s_SockAddr.sin_port = htons(s_SocketPort);

			// Creates the socket and checks if any error
			// - occured, if so we throw an runtime error
			this->s_SocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (this->s_SocketFD < 0)
			{
				std::string error = "socket() failed: ";
				error += strerror(errno);
				throw std::runtime_error(error);
			}

			// Sets some socket options, one of them is to reuse the address
			// - to prevent some stupid errors
			int32_t opt = 0x1;
			rc = setsockopt(this->s_SocketFD, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&opt), sizeof (opt));
			if (rc < 0)
			{
				std::string error = "setsockopt() failed: ";
				error += strerror(rc);
				throw std::runtime_error(error);
			}

			// Sets the socket into non-blocking mode, so we can handle stuff more
			// - fast and make the loop killable at any time
			opt = 0x1;
			rc = ioctl(this->s_SocketFD, FIONBIO, reinterpret_cast<char *>(&opt));
			if (rc < 0)
			{
				std::string error = "ioctl() failed: ";
				error += strerror(rc);
				throw std::runtime_error(error);
			}

			// Binds the socket
			rc = bind(this->s_SocketFD, reinterpret_cast<struct sockaddr *>(&this->s_SockAddr), sizeof (struct sockaddr));
			if (rc < 0)
			{
				std::string error = "bind() failed: ";
				error += strerror(rc);
				throw std::runtime_error(error);
			}
		} else throw std::runtime_error(
			"Invalid socket type supplied, must be either"
			"SMTPSocketType::SST_CLIENT or SMTPSocketType::SST_SERVER !"
		);
	}

	void SMTPSocket::asyncAcceptorThread(
		std::atomic<bool> &run,
		const std::size_t delay,
		std::atomic<bool> &running,
		const std::function<void(std::shared_ptr<struct sockaddr_in>, int32_t, void *)> &cb,
		void *u
	)
	{
		struct sockaddr_in clientSockAddr;
		int32_t clientSockFD;
		int32_t sockAddrSize = sizeof (struct sockaddr_in);

		// Sets running to true and performs infinite loop
		// - if the run variable is set to false, we will
		// - quit and set running to false
		running = true;
		while (run == true)
		{
			// Sleeps for some time ( before all the code because we may use continue )
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));

			// Accepts the client and checks for any errors, if we have any
			// error we just log it and continue
			clientSockFD = accept(this->s_SocketFD, reinterpret_cast<struct sockaddr *>(&clientSockAddr), reinterpret_cast<socklen_t *>(&sockAddrSize));
			if (clientSockFD < 0)
			{
				if (errno != EWOULDBLOCK)
					std::cout << "accept() failed: " << strerror(clientSockFD) << std::endl;					
				continue;
			}
			std::cout << "Accepted client " << std::endl;

			// Prepares the data, and then creates an new thread for the
			// - callback which will handle the request stuff ;)
			std::shared_ptr<struct sockaddr_in> sAddr = std::make_shared<struct sockaddr_in>();
			memcpy(sAddr.get(), &clientSockAddr, sizeof (struct sockaddr_in));

			std::thread t(cb, sAddr, clientSockFD, u);
			t.detach();
		}
		running = false;
	}

	void SMTPSocket::startListening(void)
	{
		int32_t rc;

		// Listens the socket and checks for errors, if there is any error
		// we will throw it immediately
		rc = listen(this->s_SocketFD, _SOCKET_MAX_IN_QUEUE);
		if (rc < 0)
		{
			std::string error = "listen() failed: ";
			error += strerror(rc);
			throw std::runtime_error(error);
		}
	}

	void SMTPSocket::startAcceptorSync(
		const std::function<void(std::shared_ptr<struct sockaddr_in>, int32_t, void *)> &cb,
		const std::size_t delay,
		const bool &mult,
		std::atomic<bool> &run,
		std::atomic<bool> &running,
		void *u
	)
	{
		std::thread t(&SMTPSocket::asyncAcceptorThread, this, std::ref(run), delay, std::ref(running), cb, u);
		t.detach();
	}

	void SMTPSocket::closeThread(std::atomic<bool> &run, std::atomic<bool> &running)
	{
		run = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		while (running) std::this_thread::sleep_for(std::chrono::milliseconds(_SOCKET_THREAD_SHUTDOWN_DELAY));
	}

	SMTPSocket::~SMTPSocket()
	{
		// Closes the socket inside of the SMTPSocket instance
		// so the memory will be free and no issues etc
		shutdown(this->s_SocketFD, SHUT_RDWR);
	}

	void SMTPSocket::sendString(int32_t &sfd, const bool &ssl, std::string &data)
	{
		int32_t rc;
		if (ssl)
		{
			return;
		}

		// Sends the string and checks for any errors
		rc = send(sfd, data.c_str(), data.length(), 0);
		if (rc < 0)
		{
			std::string error = "send() failed: ";
			error += strerror(rc);
			throw std::runtime_error(error);
		}
	}

	void SMTPSocket::receiveString(int32_t &sfd, const bool &ssl, std::string &ret)
	{
		int32_t rc;
		if (ssl)
		{
			return;
		}
	}
}