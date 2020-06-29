#include <cstdint>
#include <string>
#include <stdexcept>
#include <memory>
#include <functional>
#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <memory.h>
#include <errno.h>

#pragma once

#ifndef _SOCKET_MAX_IN_QUEUE
#define _SOCKET_MAX_IN_QUEUE 40
#endif

#ifndef _SOCKET_THREAD_SHUTDOWN_DELAY
#define _SOCKET_THREAD_SHUTDOWN_DELAY 120
#endif

namespace FSMTP::Networking
{
	typedef enum : uint8_t
	{
		SST_CLIENT = 0,
		SST_SERVER
	} SMTPSocketType;

	class SMTPSocket
	{
	public:
		SMTPSocket(const SMTPSocketType &s_SocketType,
			const int32_t &s_SocketPort);

		void startListening(void);
		void startConnecting(void);

		static void sendString(int32_t &sfd, const bool& ssl, std::string& data);
		static void receiveString(int32_t &sfd, const bool& ssl, std::string& ret);

		void startAcceptorSync(
			const std::function<void(std::shared_ptr<struct sockaddr_in>, int32_t, void *)> &cb,
			const std::size_t delay,
			const bool &mult,
			std::atomic<bool> &run,
			std::atomic<bool> &running,
			void *u
		);

		void asyncAcceptorThread(
			std::atomic<bool> &run,
			const std::size_t delay,
			std::atomic<bool> &running,
			const std::function<void(std::shared_ptr<struct sockaddr_in>, int32_t, void *)> &cb,
			void *u
		);

		void closeThread(std::atomic<bool> &run, std::atomic<bool> &running);

		void close(void);

		~SMTPSocket();
	private:
		int32_t s_SocketFD;
		int32_t s_SocketPort;
		struct sockaddr_in s_SockAddr;
		SMTPSocketType s_SocketType;
	};
}