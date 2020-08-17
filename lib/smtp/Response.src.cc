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

#include "Response.src.h"

using namespace FSMTP::SMTP;

ServerResponse::ServerResponse(const SMTPResponseType c_Type):
	c_Type(c_Type), c_Services(nullptr)
{}

ServerResponse::ServerResponse(
	const SMTPResponseType c_Type, const string &c_Message, 
	const void *c_U, vector<SMTPServiceFunction> *c_Services
):
	c_Type(c_Type), c_Services(c_Services), c_Message(c_Message),
	c_U(c_U)
{}

string ServerResponse::build(void) {
	int32_t code = ServerResponse::getCode(this->c_Type);
	ostringstream stream;

	if (this->c_Services == nullptr) {
		stream << code << ' ' << this->getMessage(this->c_Type);
		stream << " - fsmtp\r\n";
	} else {
		stream << code << '-' << this->getMessage(this->c_Type) << "\r\n";
		stream << ServerResponse::buildServices(code, this->c_Services);
	}

	return stream.str();
}

string ServerResponse::getMessage(const SMTPResponseType c_Type) {
	auto &conf = Global::getConfig();

	if (!this->c_Message.empty()) return this->c_Message;
	ostringstream stream;

	switch (c_Type) {
		case SMTPResponseType::SRC_SPAM_ALERT:
			stream << "Your address is zen.spamhaus.org database";
			break;
		case SMTPResponseType::SRC_AUTH_NOT_ALLOWED:
			stream << "Mail from different then authenticated email";
			break;
		case SMTPResponseType::SRC_DATA_START:
			stream << "End data with <CR><LF>.<CR><LF>";
			break;
		case SMTPResponseType::SRC_ORDER_ERR:
			stream << "Invalid order, why: [unknown]";
			break;
		case SMTPResponseType::SRC_INVALID_COMMAND:
			stream << "unrecognized command.";
			break;
		case SMTPResponseType::SRC_START_TLS:
			stream << "Ready to start TLS";
			break;
		case SMTPResponseType::SRC_QUIT_GOODBYE:
			stream << "Closing connection";
			break;
		case SMTPResponseType::SRC_AUTH_SUCCESS:
			stream << "Authentication successfull, welcome back.";
			break;
		case SMTPResponseType::SRC_AUTH_FAIL:
			stream << "Authentication failed, closing transmission channel.";
			break;
		case SMTPResponseType::SRC_RELAY_FAIL:
			stream << "Relay denied.";
			break;
		case SMTPResponseType::SRC_HELP_RESP:
			stream << "Fannst ESMTP server https://github.com/skywa04885/fsmtp-v2";
			break;
		case SMTPResponseType::SRC_GREETING: {
			struct tm *timeInfo = nullptr;
			char dateBuffer[128];
			std::time_t rawTime;

			time(&rawTime);
			timeInfo = localtime(&rawTime);
			strftime(
				dateBuffer,
				sizeof (dateBuffer),
				"%a, %d %b %Y %T %Z",
				timeInfo
			);

			stream << conf["node_name"].asCString();
			stream << " Fannst ESMTP Mail service ready at " << dateBuffer;
			break;
		}
		case SMTPResponseType::SRC_HELO:
		case SMTPResponseType::SRC_EHLO: {
			stream << conf["smtp"]["server"]["domain"].asCString() << ", at your service ";
			stream << DNS::getHostnameByAddress(reinterpret_cast<const struct sockaddr_in *>(this->c_U));
			stream << " [" << inet_ntoa(reinterpret_cast<const struct sockaddr_in *>(this->c_U)->sin_addr) << ']';
			break;
		}
		case SMTPResponseType::SRC_MAIL_FROM:
		case SMTPResponseType::SRC_RCPT_TO:
			stream << "OK, proceed [" << reinterpret_cast<const char *>(this->c_U) << ']';
			break;
		case SMTPResponseType::SRC_REC_NOT_LOCAL:
			stream << "User [" << reinterpret_cast<const char *>(this->c_U);
			stream << "] not local.";
			break;
		case SMTPResponseType::SRC_DATA_END:
			stream << "OK, message queued " << reinterpret_cast<const char *>(this->c_U);
			break;
		default: throw std::runtime_error("getMessage() invalid type");
	}

	return stream.str();
}

int32_t ServerResponse::getCode(const SMTPResponseType c_Type) {
	switch (c_Type) {
		case SMTPResponseType::SRC_SPAM_ALERT: return 554;
		case SMTPResponseType::SRC_GREETING: return 220;
		case SMTPResponseType::SRC_EHLO: return 250;
		case SMTPResponseType::SRC_HELO: return 250;
		case SMTPResponseType::SRC_MAIL_FROM: return 250;
		case SMTPResponseType::SRC_RCPT_TO: return 250;
		case SMTPResponseType::SRC_DATA_START: return 354;
		case SMTPResponseType::SRC_DATA_END: return 250;
		case SMTPResponseType::SRC_QUIT_GOODBYE: return 221;
		case SMTPResponseType::SRC_SYNTAX_ERR: return 501;
		case SMTPResponseType::SRC_ORDER_ERR: return 503;
		case SMTPResponseType::SRC_INVALID_COMMAND: return 502;
		case SMTPResponseType::SRC_START_TLS: return 220;
		case SMTPResponseType::SRC_REC_NOT_LOCAL: return 551;
		case SMTPResponseType::SRC_AUTH_SUCCESS: return 235;
		case SMTPResponseType::SRC_AUTH_FAIL: return 530;
		case SMTPResponseType::SRC_RELAY_FAIL: return 551;
		case SMTPResponseType::SRC_HELP_RESP: return 214;
		case SMTPResponseType::SRC_AUTH_NOT_ALLOWED: return 551;
		default: throw std::runtime_error("getCode() invalid type");
	}
}

string ServerResponse::buildServices(
	const int32_t code,
	vector<SMTPServiceFunction> *c_Services
) {
	size_t index = 0, total = c_Services->size();
	ostringstream stream;

	for_each(c_Services->begin(), c_Services->end(), [&](auto &service) {
		stream << code << (++index == total ? ' ' : '-') << service.s_Name;
		for_each(service.s_SubArgs.begin(), service.s_SubArgs.end(), [&](auto &arg) {
			stream << ' ' << arg;
		});
		stream << "\r\n";
	});

	return stream.str();
}

tuple<int32_t, string> ServerResponse::parseResponse(const string &raw) {
	string clean;
	reduceWhitespace(raw, clean);

	size_t index = clean.find_first_of(' ');
	if (index == string::npos)
		return make_pair(stoi(clean), "");
	else {
		return make_pair(stoi(clean.substr(0, index)), clean.substr(index + 1));
	}
}