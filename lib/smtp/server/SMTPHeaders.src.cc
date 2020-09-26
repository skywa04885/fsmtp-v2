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

#include "SMTPHeaders.src.h"

namespace FSMTP::SMTP::Server::Headers {
  string buildReceived(
    const string &from, const string &fromAddr,
    const string &mailFrom, const string &spf,
    int32_t port
  ) {
    char dateBuffer[64];
    time_t rawTime;
    struct tm *timeinfo = nullptr;
    ostringstream result;

    // Gets the current time, this is used to keep track
    //  of when it hit which server
    time(&rawTime);
    timeinfo = localtime(&rawTime);
    strftime(dateBuffer, sizeof (dateBuffer), "%a, %-d %b %Y %T (%Z)", timeinfo);

    // Formats the received header, not made for human readability
    result << "from " << from << "([" << fromAddr << "]:" << port << ") by " << by << " with ESMTP (FSMTP-V2 By Luke Rieff) ";
    result << "(spf:" << spf << ")(envelope-from:<" << mailFrom << ">); " << dateBuffer;

    return result.str();
  }
};