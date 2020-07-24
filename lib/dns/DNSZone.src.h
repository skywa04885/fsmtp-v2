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

	typedef enum : uint8_t
	{
		REC_CLASS_IDK = 0
	} RecordClass;

	class Record
	{
	public:
		/**
		 * Default empty constructor of an record
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		explicit Record(void);

		/**
		 * The constructor for an record
		 *
		 * @Param {const char *} r_Data
		 * @Param {const char *} r_Root
		 * @Param {const std::size_t} r_DatLen
		 * @Param {const int32_t} r_TTL,
		 * @Param {const RecordType} r_Type
		 * @Param {const RecordClass r_Class}
		 * @Return {void}
		 */
		Record(const char *r_Data, const char *r_Root,
			const std::size_t r_DataLen, const int32_t r_TTL,
			const RecordType r_Type, const RecordClass r_Class);
	private:
		const char *r_Data;
		const char *r_Root;
		std::size_t r_DataLen;
		int32_t r_TTL;
		RecordType r_Type;
		RecordClass r_Class;
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