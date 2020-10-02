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

#ifndef _LIB_DNS_RESOLVER_H
#define _LIB_DNS_RESOLVER_H

#include "../default.h"
#include "../general/Logger.src.h"

namespace FSMTP::DNS {
	class RR {
	public:
		RR(int32_t ttl, int32_t cl, int32_t type, string &&name, string &&data);
		
		int32_t getTTL() const;
		int32_t getClass() const;
		int32_t getType() const;
		const string &getName() const;
		const string &getData() const;

		static void print(Logger &logger, const vector<RR> &records);
		
		~RR();
	private:
		int32_t m_TTL, m_Class, m_Type;
		string m_Name, m_Data;
	};

	class Resolver {
	public:
		Resolver();

		Resolver &query(const char *query, int32_t type);
		Resolver &initParse();
		vector<RR> getRecords();
		vector<RR> getTXTRecords();
		Resolver &reset();

		~Resolver();
	private:
		int32_t m_AnswerLen, m_ResponseCount;
		u_char m_Buffer[8192];
		struct __res_state m_State;
		ns_msg m_NsMsg;
	};

	string resolveHostname(const string &hostname);
	
	template<typename T>
	string getHostnameByAddress(const T *a) {
		char hostname[512];
		getnameinfo(reinterpret_cast<const struct sockaddr *>(a), 
			sizeof (T), hostname,
			sizeof (hostname), nullptr, 0, NI_NAMEREQD);
		return hostname;
	}
	vector<string> resolveAllFromHostname(const string &hostname);
}

#endif
