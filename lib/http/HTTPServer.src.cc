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

#include "HTTPServer.src.h"

namespace FSMTP::HTTP {
	HTTPServer::HTTPServer():
		m_Logger("HTTPServer", LoggerLevel::DEBUG)
	{}

	HTTPServer &HTTPServer::listenServer() {
		auto &conf = Global::getConfig();
		auto &logger = this->m_Logger;

		int32_t httpsPort = conf["ports"]["https"].asInt();
		int32_t httpPort = conf["ports"]["http"].asInt();

		// ============================
		// Uses IPv6 if enabled
		// ============================

		if (conf["ipv6"].asBool()) {
			this->m_HTTPS = make_unique<ServerSocket>(ServerSocketAddrType::ServerSocketAddr_IPv6);
			this->m_HTTP = make_unique<ServerSocket>(ServerSocketAddrType::ServerSocketAddr_IPv6);

			this->m_HTTPS->queue(250).useSSL(this->m_SSLContext.get()).listenServer(httpsPort);
			this->m_HTTP->queue(250).listenServer(httpPort);
			logger << "IPv6 Listening on { HTTPS: " << httpsPort << ", HTTP: " << httpPort << " }" << ENDL;
		}

		// ============================
		// Uses IPv4 if IPv6 disabled 
		// ============================

		if (!conf["ipv6"].asBool()) {
			this->m_HTTPS = make_unique<ServerSocket>(ServerSocketAddrType::ServerSocketAddr_IPv4);
			this->m_HTTP = make_unique<ServerSocket>(ServerSocketAddrType::ServerSocketAddr_IPv4);

			this->m_HTTPS->queue(250).useSSL(this->m_SSLContext.get()).listenServer(httpsPort);
			this->m_HTTP->queue(250).listenServer(httpPort);
			logger << "IPv4 Listening on { HTTPS: " << httpsPort << ", HTTP: " << httpPort << " }" << ENDL;
		}

		return *this;
	}

	HTTPServer &HTTPServer::createContext() {
		this->m_SSLContext = Global::getSSLContext(SSLv23_server_method());
		return *this;
	}

	HTTPServer &HTTPServer::startHandler(bool newThread) {
		auto &logger = this->m_Logger;

		auto handler = [&](shared_ptr<ClientSocket> client) {
			Logger clogger("HTTP[" + client->getPrefix() + ']', LoggerLevel::DEBUG);
			DEBUG_ONLY(clogger << "Client connected .." << ENDL);

			// Attempts to parse the HTTP request, and respond with an valid
			//  response, if this fails we catch it instead of crashing the server
			try {
				HTTPRequest request;

				// Gets the initial request with the headers, after this we will
				//  parse it, and check if we should also read an body, if it is
				//  not the case, just respond
				request.parse(client->readToDelim("\r\n\r\n"));
				request.print(clogger);

				// Sends the test response
				HTTPResponse resp(request, client);
				resp.sendText(200, request.getUserAgent(), MIME::FileTypes::HypertextMarkupLanguage, HTTPCharset::UTF8);
			} catch (const runtime_error &e) {
				clogger << ERROR << "An runtime earror occured: '" << e.what() << '\'' << ENDL;
			} catch (...) {
				clogger << ERROR << "An unknown error occured !" << ENDL;
			}

			DEBUG_ONLY(clogger << "Client disconnected .." << ENDL);
		};

		this->m_HTTPS->handler(handler).startAcceptor(true);
		this->m_HTTP->handler(handler).startAcceptor(newThread);
		logger << INFO << "Handlers started .." << ENDL << CLASSIC;

		return *this;
	}

	HTTPServer::~HTTPServer() = default;
}
