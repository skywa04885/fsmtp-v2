#ifndef _LIB_DMARC_H
#define _LIB_DMARC_H

#include "../default.h"

namespace FSMTP::DMARC {
	typedef enum {
		DIT_EMAIL_ADDRESS
	} DMARCInformType;

	typedef struct {
		string t_Data;
		DMARCInformType t_Type;
	} DMARCInformTarget;

	class DMARCRecord {
	public:
		DMARCRecord() noexcept;
	private:
		uint8_t r_Flags;
		vector<DMARCInformType> r_FailReportTargets;
		vector<DMARCInformType> r_FeronsicReportTargets;
	};
}

#endif
