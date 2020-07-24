#include "DNSZone.src.h"

namespace FSMTP::DNS
{
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
	Record::Record(const char *r_Data, const char *r_Root,
		const std::size_t r_DataLen, const int32_t r_TTL,
		const RecordType r_Type, const RecordClass r_Class
	):
		r_Data(r_Data), r_Root(r_Root), r_TTL(r_TTL), r_Type(r_Type),
		r_Class(r_Class)
	{
		// Calculates the data length of the rData field
		this->r_DataLen = strlen(this->r_Data);
	}
}