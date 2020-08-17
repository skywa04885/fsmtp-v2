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

#include "../default.h"

#include "../general/Logger.src.h"

namespace FSMTP::DNS {
	typedef enum : uint8_t {
		RT_MX = 0,
		RT_TXT
	} RecordType;

	typedef struct {
		string r_Value;
		string r_Name;
		int32_t r_Type;
		int32_t r_TTL;
		int32_t r_Class;
		int32_t r_ReadLen;
	} Record;

	string resolveHostname(const char *hostname);
	string getHostnameByAddress(const struct sockaddr_in *addr);

	vector<Record> resolveDNSRecords(const string &hostname, const RecordType type);
}
