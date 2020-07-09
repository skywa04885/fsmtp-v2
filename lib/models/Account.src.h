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

#include <cstdint>
#include <string>
#include <memory>

#include <cassandra.h>

#include "../general/connections.src.h"
#include "../general/exceptions.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
	class Account
	{
	public:
	private:
		std::string a_Domain;
		std::string a_Username;
		std::string a_FullName;
		std::string a_Password;
		int64_t a_Bucket;
	};

	class AccountShortcut
	{
	public:
		AccountShortcut(
			int64_t a_Bucket,
			std::string a_Domain,
			std::string a_Username,
			CassUuid a_UUID
		);

		explicit AccountShortcut();

		static void find(
			std::unique_ptr<CassandraConnection> &conn,
			AccountShortcut &shortcut,
			const std::string &domain,
			const std::string &username
		);

		void save(std::unique_ptr<CassandraConnection> &conn);

		int64_t a_Bucket;
		std::string a_Domain;
		std::string a_Username;
		CassUuid a_UUID;
	};
}