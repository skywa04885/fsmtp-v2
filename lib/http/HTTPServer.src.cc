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
			client->timeout(1);
			Logger clogger("HTTP[" + client->getPrefix() + ']', LoggerLevel::DEBUG);
			DEBUG_ONLY(clogger << "Client connected .." << ENDL);

			// Attempts to parse the HTTP request, and respond with an valid
			//  response, if this fails we catch it instead of crashing the server
			try {
				HTTPRequest request;
				request.parse(client->readToDelim("\r\n\r\n"));
				DEBUG_ONLY(request.print(clogger));
				
				// Sends the test response
				HTTPResponse response(request, client);
				defaultCallback(request, response, client);
			} catch (const runtime_error &e) {
				clogger << ERROR << "An runtime earror occured: '" << e.what() << '\'' << ENDL;
			} 

			DEBUG_ONLY(clogger << "Client disconnected .." << ENDL);
		};

		this->m_HTTPS->handler(handler).startAcceptor(true);
		this->m_HTTP->handler(handler).startAcceptor(newThread);
		logger << INFO << "Handlers started .." << ENDL << CLASSIC;

		return *this;
	}

	void HTTPServer::defaultCallback(HTTPRequest request, HTTPResponse response, shared_ptr<Sockets::ClientSocket> client) {
		auto &config = Global::getConfig();

		nlohmann::json body;
		body["client"]["port"] = client->getPort();
		body["client"]["address"] = client->getPrefix();
		body["client"]["protocol"] = (client->getRealProtocol() == Networking::IP::Protocol::Protocol_IPv4 ? "IPv4" : "IPv6");
		
		body["server"]["node"] = config["node_name"].asString();
		body["server"]["domain"] = config["domain"].asString();
		body["server"]["ipv6"] = config["ipv6"].asBool();

		body["server"]["response"]["code"] = 404;
		body["server"]["response"]["message"] = "No service registered to hostname / path";

		body["server"]["request"]["user-agent"] = request.getUserAgent();
		body["server"]["request"]["connection"] = __httpConnectionToString(request.getConnection());
		body["server"]["request"]["version"] = __httpVersionToString(request.getVersion());
		body["server"]["request"]["method"] = __httpMethodToString(request.getMethod());
		body["server"]["request"]["uri"]["path"] = request.getURI().path;
		body["server"]["request"]["uri"]["hostname"] = request.getURI().hostname;
		body["server"]["request"]["uri"]["search"] = request.getURI().search;

		auto &headers = request.getHeaders();
		for_each(headers.begin(), headers.end(), [&](const MIME::MIMEHeader &h) {
			body["server"]["request"]["headers"][h.key] = h.value;
		});

		response.sendText(404, body.dump(), MIME::FileTypes::JSON, HTTPCharset::UTF8);
	}

	HTTPServer::~HTTPServer() = default;
}
