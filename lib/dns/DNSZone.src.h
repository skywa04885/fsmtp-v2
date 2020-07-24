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

#include "DNS.src.h"

namespace FSMTP::DNS
{
	typedef enum : uint8_t
	{
		REC_TYPE_A = 0,
		REC_TYPE_AAAA,
		REC_TYPE_ALIAS,
		REC_TYPE_CNAME,
		REC_TYPE_MX,
		REC_TYPE_NS,
		REC_TYPE_PTR,
		REC_TYPE_SOA,
		REC_TYPE_SRV,
		REC_TYPE_TXT,
		REC_TYPE_UNKNOWN
	} RecordType;

	class Record
	{
	public:
	private:
		const char *r_Content;
		const char *r_Root;
		RecordType r_Type;
		int32_t r_TTL;
	};

	class Domain
	{
	public:
		/**
		 * Default empty constructor for domain
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit Domain(void);
	private:
		const char *d_Domain;
		std::vector<Record> d_Records;
	};

	class Zone
	{
	public:
	private:
	};
}