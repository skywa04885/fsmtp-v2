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

#include "LocalDomain.src.h"

namespace FSMTP::Models
{
	/**
	 * Creates an new local domain and generates
	 * the uuid automatically for it
	 * 
	 * @Param {std::string} l_Domain
	 * @Return void
	 */
	LocalDomain::LocalDomain(const std::string &l_Domain):
		l_Domain(l_Domain)
	{
		// Creates the UUID gen and generates an new 
		// - user id for the domain
		CassUuidGen *uuidGen = cass_uuid_gen_new();
		cass_uuid_gen_time(uuidGen, &this->l_UUID);
		cass_uuid_gen_free(uuidGen);
	}

	/**
	 * Empty constructor, will just initialize the values

	 * @Param void
	 * @Return void
	 */
	LocalDomain::LocalDomain(void):
		l_Domain()
	{}

	/**
	 * Searches in the database for an domain with that
	 * specific ID
	 *
	 * @Param {const std::string &} l_Domain
	 * @Param {std::unique_ptr<CassandraConnection> &} database
	 * @Return void
	 */
	void LocalDomain::getByDomain(
		const std::string &l_Domain,
		std::unique_ptr<CassandraConnection>& database
	)
	{

	}
}