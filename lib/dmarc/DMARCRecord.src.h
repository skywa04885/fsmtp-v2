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

#ifndef _LIB_DMARC_H
#define _LIB_DMARC_H

#include "../default.h"
#include "../general/Logger.src.h"
#include "../dns/Resolver.src.h"

#define _FSMTP_DMARC_REPORT_INTERVAL_DEFAULT 86400
#define _FSMTP_DMARC_REPORT_FILTER_PERC_DEFAULT 0

namespace FSMTP::DMARC {
	typedef enum {
		InformMailto
	} DMARCInformType;

	const char *__dmarcInformTypeToString(DMARCInformType t);

	typedef enum {
		PolicyNone, PolicyQuarantine, PolicyReject
	} DMARCPolicy;

	const char *__dmarcPolicyToString(DMARCPolicy p);

	typedef enum {
		DMARC1
	} DMARCVersion;

	const char *__dmarcVersionToString(DMARCVersion v);

	typedef enum {
		AlignmentRelaxed, AlignmentStrict
	} DMARCAlignment;

	const char *__dmarcAlignmentToString(DMARCAlignment a);

	typedef enum {
		FormatAFRF
	} DMARCReportFormat;

	const char *__dmarcReportFormatToString(DMARCReportFormat f);

	typedef enum {
		ReportOpt_GenerateIfAllFail,
		ReportOpt_GenerateIfAnyFail,
		ReportOpt_GenerateAlwaysDKIMFail,
		ReportOpt_GenerateAlwaysSPFFail
	} DMARCReportOptions;

	const char *__dmarcReportOptionsToString(DMARCReportOptions o);

	typedef struct {
		string t_Data;
		DMARCInformType t_Type;
	} DMARCInformTarget;

	class DMARCRecord {
	public:
		DMARCRecord() noexcept;

		DMARCRecord &parse(const string &raw);
		DMARCRecord &print(Logger &logger);

		const char *getPolicyString();
		const char *getSubdomainPolicyString();
		const char *getDKIMAlignmentString();
		const char *getSPFAlignmentString();
		const char *getReportFormatString();
		const char *getVersionString();
		const char *getReportOptionsString();

		static DMARCRecord fromDNS(const char *query);

		~DMARCRecord() noexcept;
	private:
		uint8_t m_Flags;
		DMARCPolicy m_Policy, m_SubdomainPolicy;
		DMARCAlignment m_DKIMAlignment, m_SPFAlignment;
		DMARCVersion m_Version;
		DMARCReportFormat m_ReportFormat;
		DMARCReportOptions m_ReportOptions;
		int32_t m_FilteringPercentage;
		uint64_t m_ReportInterval;
		vector<DMARCInformTarget> m_FailReportTargets, m_FeronsicReportTargets;
	};
}

#endif
