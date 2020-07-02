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

#include <memory>
#include <iostream>

#include <cassandra.h>

#include "../general/connections.src.h"
#include "../general/exceptions.src.h"

using namespace FSMTP::Connections;

namespace FSMTP::Models
{
	class LocalDomain
	{
	public:
		/**
		 * Creates an new local domain and generates
		 * the uuid automatically for it
		 * 
		 * @Param {std::string} l_Domain
		 * @Return void
		 */
		LocalDomain(const std::string &l_Domain);

		/**
		 * Empty constructor, will just initialize the values
		 
		 * @Param void
		 * @Return void
		 */
		LocalDomain(void);

		/**
		 * Searches in the database for an domain with that
		 * specific ID
		 *
		 * @Param {const std::string &} l_Domain
		 * @Param {std::unique_ptr<CassandraConnection> &} database
		 * @Return void
		 */
		void getByDomain(const std::string &l_Domain, std::unique_ptr<CassandraConnection>& database);
	private:
		std::string l_Domain;
		CassUuid l_UUID;
	};
}