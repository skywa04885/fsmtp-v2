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

#ifndef _LIB_SMTP_SERVER_HEADERS_H
#define _LIB_SMTP_SERVER_HEADERS_H

#include "../../default.h"
#include "../../general/Global.src.h"

namespace FSMTP::SMTP::Server::Headers {
  string buildReceived(
    const string &from, const string &fromAddr,
    const string &mailFrom, const string &spf,
    int32_t port 
  );
};

#endif
