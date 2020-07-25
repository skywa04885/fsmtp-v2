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
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <atomic>
#include <algorithm>
#include <tuple>
#include <sstream>
#include <json/json.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <sys/sysinfo.h>

#include "../networking/DNS.src.h"
#include "../general/Logger.src.h"
#include "../general/macros.src.h"
#include "../general/cleanup.src.h"
#include "../models/Email.src.h"
#include "../general/connections.src.h"
#include "../models/LocalDomain.src.h"
#include "../models/Account.src.h"
#include "../general/Passwords.src.h"
#include "../models/RawEmail.src.h"
#include "../models/EmailShortcut.src.h"
#include "../parsers/mime.src.h"
#include "../general/exceptions.src.h"

using namespace FSMTP::Cleanup;
using namespace FSMTP::Parsers;
