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

		for (int32_t i = 0; i < this->m_ResponseCount; ++i) {
			ns_parserr(&this->m_NsMsg, ns_s_an, i, &record);

			string raw(reinterpret_cast<const char *>(ns_rr_rdata(record) + 1), ns_rr_rdlen(record) - 1);
			string data;
			data.reserve(raw.length());
			for (char c : raw) {
				if (c >= 0 && c <= 127) data += c;
			}
			
			result.push_back(RR(
				ns_rr_ttl(record), ns_rr_class(record), ns_rr_type(record),
				ns_rr_name(record), move(data)
			));
		}

		return result;
	}

	Resolver &Resolver::reset() {
		this->m_Buffer[0] = '\0';
		this->m_ResponseCount = 0;
		this->m_AnswerLen = 0;
		return *this;
	}

	Resolver::~Resolver() {
		res_nclose(&this->m_State);
	}

	string resolveHostname(const string &hostname, int32_t af) {
		struct hostent *h = nullptr;
		if ((h = gethostbyname2(hostname.c_str(), af)) == nullptr)
			throw runtime_error("Could not resolve hostname: " + hostname);
		
		struct in_addr *p = reinterpret_cast<struct in_addr *>(h->h_addr);

		// Checks AF, if it is AF_INET we will get the IPv4 address
		//  else we will get the IPv6 address string, else runtime_error
		switch (af) {
			case AF_INET: {
				char buffer[INET_ADDRSTRLEN];

				// Turns the IPv4 address into an string, if anything goes
				//  wrong throw error
				if (inet_ntop(AF_INET, p, buffer, INET_ADDRSTRLEN) == nullptr)
					throw runtime_error(EXCEPT_DEBUG(
						string("inet_ntop() failed: ") + strerror(errno)));

				return buffer;
			}
			case AF_INET6: {
				char buffer[INET6_ADDRSTRLEN];

				// Turns the IPv6 address into an string, if anything goes
				//  wrong, throw an error
				if (inet_ntop(AF_INET6, p, buffer, INET6_ADDRSTRLEN) == nullptr)
					throw runtime_error(EXCEPT_DEBUG(
						string("inet_ntop() failed: ") + strerror(errno)));
				
				return buffer;
			}
			default: throw invalid_argument("af needs to be either AF_INET or AF_INET6");
		}
	}

	vector<string> resolveAllFromHostname(const string &hostname, int32_t af) {
		vector<string> result = {};

		struct hostent *h = nullptr;
		if ((h = gethostbyname2(hostname.c_str(), af)) == nullptr)
			throw runtime_error("Could not resolve jostname" + hostname);

		struct in_addr **p = reinterpret_cast<struct in_addr **>(h->h_addr_list);
		for (; *p != nullptr; ++p) result.push_back(inet_ntoa(**p));

		return result;
	}
}
