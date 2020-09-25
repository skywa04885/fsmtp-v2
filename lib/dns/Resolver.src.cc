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

#include "Resolver.src.h"

namespace FSMTP::DNS {
	RR::RR(int32_t ttl, int32_t cl, int32_t type, string &&name, string &&data):
		m_TTL(ttl), m_Class(cl), m_Type(type), m_Name(name), m_Data(data)
	{}

	int32_t RR::getTTL() const {
		return this->m_TTL;
	}

	int32_t RR::getClass() const {
		return this->m_Class;
	}
	
	int32_t RR::getType() const {
		return this->m_Type;
	}

	const string &RR::getName() const {
		return this->m_Name;
	}

	const string &RR::getData() const {
		return this->m_Data;
	}

	void RR::print(Logger &logger, const vector<RR> &records) {
		logger << DEBUG;
		logger << "Records: " << ENDL;

		size_t i = 0;
		for_each(records.begin(), records.end(), [&](const RR &r) {
			logger << '\t' << i++ << " -> { TTL: " << r.getTTL()
				<< ", Class: " << r.getClass()
				<< ", Type: " << r.getType()
				<< ", Name: '" << r.getName() 
				<< "', Data '" << r.getData() << "' }" << ENDL;
		});

		logger << CLASSIC;
	}

	RR::~RR() = default;

	Resolver::Resolver() {
		res_ninit(&this->m_State);
	}

	Resolver &Resolver::query(const char *query, int32_t type) {
		if ((this->m_AnswerLen = res_nquery(&this->m_State, query, ns_c_in, 
			type, this->m_Buffer, sizeof (this->m_Buffer))
		) < 0) throw runtime_error("Query has no results");

		return *this;
	}

	Resolver &Resolver::initParse() {
		ns_initparse(this->m_Buffer, this->m_AnswerLen, &this->m_NsMsg);
		this->m_ResponseCount = ns_msg_count(this->m_NsMsg, ns_s_an);
		return *this;
	}

	vector<RR> Resolver::getRecords() {
		vector<RR> result = {};
		ns_rr record;
		char data[2048];

		for (int32_t i = 0; i < this->m_ResponseCount; ++i) {
			ns_parserr(&this->m_NsMsg, ns_s_an, i, &record);
			dn_expand(ns_msg_base(this->m_NsMsg), ns_msg_end(this->m_NsMsg),
				ns_rr_rdata(record) + 2, data, sizeof (data));
			
			string s(reinterpret_cast<const char *>(ns_rr_rdata(record) + 1), ns_rr_rdlen(record) - 1);
			cout << s << endl;

			result.push_back(RR(
				ns_rr_ttl(record), ns_rr_class(record), ns_rr_type(record),
				ns_rr_name(record), data
			));
		}

		return result;
	}

		vector<RR> Resolver::getTXTRecords() {
		vector<RR> result = {};
		ns_rr record;
		char data[2048];

		for (int32_t i = 0; i < this->m_ResponseCount; ++i) {
			ns_parserr(&this->m_NsMsg, ns_s_an, i, &record);
			dn_expand(ns_msg_base(this->m_NsMsg), ns_msg_end(this->m_NsMsg),
				ns_rr_rdata(record) + 2, data, sizeof (data));
			
			result.push_back(RR(
				ns_rr_ttl(record), ns_rr_class(record), ns_rr_type(record),
				ns_rr_name(record), string(reinterpret_cast<const char *>(ns_rr_rdata(record) + 1), ns_rr_rdlen(record) - 1)
			));
		}

		return result;
	}

	Resolver::~Resolver() {
		res_nclose(&this->m_State);
	}

	string resolveHostname(const string &hostname) {
		struct hostent *h = nullptr;
		if ((h = gethostbyname(hostname.c_str())) == nullptr)
			throw runtime_error("Could not resolve hostname: " + hostname);

		struct in_addr *p = reinterpret_cast<struct in_addr *>(h->h_addr);
		return inet_ntoa(*p);
	}

	string getHostnameByAddress(const struct sockaddr_in *a) {
		char hostname[512];
		getnameinfo(reinterpret_cast<const struct sockaddr *>(a), 
			sizeof (struct sockaddr), hostname,
			sizeof (hostname), nullptr, 0, NI_NAMEREQD);
		return hostname;
	}
}
