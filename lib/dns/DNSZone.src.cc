#include "DNSZone.src.h"

extern Json::Value _config;

namespace FSMTP::DNS
{
	/**
	 * Default empty constructor for domain
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	Domain::Domain(void)
	{}

	/**
	 * Logs the domain to the console
	 *
	 * @Param {Logger &} logger
	 * @Return {void}
	 */
	void Domain::log(Logger &logger)
	{
		logger << DEBUG;

		logger << "[Domain]: " << ENDL;
		logger << "- Domain: " << this->d_Domain << ENDL;
		logger << "- Records[type: A, len: " << this->d_ARecords.size() << "]: " << ENDL;
		for (const DNSRecord &record : this->d_ARecords)
		{
			logger << "\tRecord[type: A]: " << ENDL;
			logger << "\t- Data: " << record.r_Data << ENDL;
			logger << "\t- DataLen: " << record.r_Data.size() << ENDL;
			logger << "\t- Root: " << record.r_Root << ENDL;
			logger << "\t- TTL: " << record.r_TTL << ENDL;
			logger << "\t- Class: " << record.r_Class << ENDL;
			logger << "\t- Type: " << record.r_Type << ENDL;
		}

		logger << CLASSIC;
	}

	/**
	 * The constructor for an record
	 *
	 * @Param {const std::string &} r_Data
	 * @Param {const std::string &} r_Root
	 * @Param {const int32_t} r_TTL,
	 * @Param {const ResponseRecordType} r_Type
	 * @Param {const QueryClass r_Class}
	 * @Return {void}
	 */
	DNSRecord::DNSRecord(const std::string &r_Data, const std::string &r_Root,
		const int32_t r_TTL, const ResponseRecordType r_Type, 
		const QueryClass r_Class):
		r_Data(r_Data), r_Root(r_Root), r_TTL(r_TTL), r_Type(r_Type),
		r_Class(r_Class)
	{}

	/**
	 * Builds an response record
	 *
	 * @Param {char *} ret
	 * @Return {std::size_t}
	 */
	std::size_t DNSRecord::build(char *ret) const
	{
		std::size_t retIndex = 0;

		// Adds the query index
		int16_t namePtr = 0b1100000000000000;
		namePtr |= htons(12);
		memcpy(reinterpret_cast<void *>(&ret[retIndex]), &namePtr, sizeof (uint16_t));
		retIndex += sizeof (uint16_t);

		// Sets the record type
		int16_t recordTypeCode = htons(responseRecordTypeToInt(this->r_Type));
		memcpy(reinterpret_cast<void *>(&ret[retIndex]), &recordTypeCode, sizeof (int16_t));
		retIndex += sizeof (int16_t);

		// Sets the record class
		int16_t recordClassCode = htons(queryClassToInt(this->r_Class));
		memcpy(reinterpret_cast<void *>(&ret[retIndex]), &recordTypeCode, sizeof (int16_t));
		retIndex += sizeof (int16_t);

		// Adds the TTL
		uint32_t ttl = htons(this->r_TTL);
		memcpy(reinterpret_cast<void *>(&ret[retIndex]), &ttl, sizeof (uint32_t));
		retIndex += sizeof (uint32_t);

		// Adds the RD len
		uint16_t dataLen = htons(4);
		memcpy(reinterpret_cast<void *>(&ret[retIndex]), &dataLen, sizeof (uint16_t));
		retIndex += sizeof (uint16_t);

		// Adds the address, and splits it on the dots
		std::stringstream s(this->r_Data);
		std::string tok;
		while (std::getline(s, tok, '.'))
		{
			int addrNumInt = std::atoi(tok.c_str());
			uint8_t addrNum = static_cast<uint8_t>(addrNumInt);
			memcpy(reinterpret_cast<void *>(&ret[retIndex]), &addrNum, 1);
			retIndex++;
		}

		return retIndex;
	}

	/**
	 * Turns an response record type back into an integer
	 *
	 * @Param {const ResponseRecordType} type
	 * @Return {int16_t}
	 */
	int16_t responseRecordTypeToInt(const ResponseRecordType type)
	{
		switch (type)
		{
			case ResponseRecordType::REC_TYPE_A: return 0;
			case ResponseRecordType::REC_TYPE_AAAA: return 18;
			case ResponseRecordType::REC_TYPE_MX: return 15;
			case ResponseRecordType::REC_TYPE_SOA: return 6;
			case ResponseRecordType::REC_TYPE_TXT: return 16;
			default: case ResponseRecordType::REC_TYPE_UNKNOWN: return 255;
		}
	}

	/**
	 * Reads the zone from the config
	 *
	 * @Param {void}
	 * @Return {std::vector<Domain>}
	 */
	std::vector<Domain> readConfig(void)
	{
		Logger logger("DNSCFG", LoggerLevel::INFO);
		std::vector<Domain> result = {};

		// Starts reading the zones
		logger << "Reading configuration zones ..." << ENDL;
		Json::Value defaultValue;
		for (std::size_t i = 0; i < _config["zone"].size(); i++)
		{
			Json::Value val = _config["zone"].get(i, defaultValue);
			Domain domain;
			domain.d_Domain = val["domain"].asString();

			// Gets the records
			for (std::size_t j = 0; j < val["records"]["a"].size(); j++)
			{
				Json::Value record = val["records"]["a"].get(j, defaultValue);
				domain.d_ARecords.push_back(DNSRecord(
					record["record_data"].asString(),
					record["record_root"].asString(),
					record["record_ttl"].asInt(),
					ResponseRecordType::REC_TYPE_A,
					QueryClass::QUERY_CLASS_INTERNET
				));
			}

			// Prints the domain, and pushes it to the result
			domain.log(logger);
			result.push_back(domain);
		}

		return result;
	}
}