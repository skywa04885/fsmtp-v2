#include "DMARCRecord.src.h"

namespace FSMTP::DMARC {
	const char *__dmarcInformTypeToString(DMARCInformType t) {
		switch (t) {
			case DMARCInformType::InformMailto: return "mailto";
		}
	}

	const char *__dmarcPolicyToString(DMARCPolicy p) {
		switch (p) {
			case DMARCPolicy::PolicyNone: return "none";
			case DMARCPolicy::PolicyQuarantine: return "quarantine";
			case DMARCPolicy::PolicyReject: return "reject";
		}
	}

	const char *__dmarcAlignmentToString(DMARCAlignment a) {
		switch (a) {
			case DMARCAlignment::AlignmentRelaxed: return "relaxed";
			case DMARCAlignment::AlignmentStrict: return "strict";
		}
	}

	const char *__dmarcReportFormatToString(DMARCReportFormat f) {
		switch (f) {
			case DMARCReportFormat::FormatAFRF: return "AFRF";
		}
	}

	const char *__dmarcVersionToString(DMARCVersion v) {
		switch (v) {
			case DMARCVersion::DMARC1: return "DMARC1";
		}
	}

	const char *__dmarcReportOptionsToString(DMARCReportOptions o) {
		switch (o) {
			case DMARCReportOptions::ReportOpt_GenerateAlwaysSPFFail: return "Report always if SPF fails";
			case DMARCReportOptions::ReportOpt_GenerateAlwaysDKIMFail: return "Report always if DKIM fails";
			case DMARCReportOptions::ReportOpt_GenerateIfAllFail: return "Report if all fail";
			case DMARCReportOptions::ReportOpt_GenerateIfAnyFail: return "Report if any fails";
		}
	}

	DMARCRecord::DMARCRecord() noexcept:
		m_Flags(0x0), m_SubdomainPolicy(DMARCPolicy::PolicyNone),
		m_Policy(DMARCPolicy::PolicyNone), m_Version(DMARCVersion::DMARC1), m_DKIMAlignment(DMARCAlignment::AlignmentRelaxed),
		m_SPFAlignment(DMARCAlignment::AlignmentRelaxed), m_ReportFormat(DMARCReportFormat::FormatAFRF),
		m_ReportInterval(_FSMTP_DMARC_REPORT_INTERVAL_DEFAULT), m_FilteringPercentage(_FSMTP_DMARC_REPORT_FILTER_PERC_DEFAULT),
		m_ReportOptions(DMARCReportOptions::ReportOpt_GenerateIfAllFail)
	{}

	DMARCRecord &DMARCRecord::parse(const string &raw) {
		#ifdef _SMTP_DEBUG
		Logger logger("DMARCRecord::parse", LoggerLevel::DEBUG);
		logger << "Parsing record: '" << raw << '\'' << ENDL;
		#endif

		// ==================================
		// Splits the record into segments
		// ==================================

		size_t start = 0, end = raw.find_first_of(';');
		vector<string> segments = {};

		for (;;) {
			segments.push_back(raw.substr(start, end - start));
			if (end == string::npos) break;

			start = end + 1;
			end = raw.find_first_of(';', start);
		}

		// Prints the segments for debug purposes
		#ifdef _SMTP_DEBUG
		for_each(segments.begin(), segments.end(), [&](const string &seg) {
			logger << "Found segment: '" << seg << '\'' << ENDL;
		});
		#endif

		// ==================================
		// Makes sense of the segments
		// ==================================

		auto parsePolicy = [](const string &p) {
			if (p == "reject") return DMARCPolicy::PolicyReject;
			else if (p == "quarantine") return DMARCPolicy::PolicyQuarantine;
			else if (p == "none") return DMARCPolicy::PolicyNone;
			else return DMARCPolicy::PolicyNone;
		};

		auto parseAlignment = [](const string &a) {
			if (a == "s") return DMARCAlignment::AlignmentStrict;
			else if (a == "r") return DMARCAlignment::AlignmentRelaxed;
			else DMARCAlignment::AlignmentRelaxed;
		};

		auto parseFormat = [](const string &f) {
			if (f == "afrf") return DMARCReportFormat::FormatAFRF;
			else return DMARCReportFormat::FormatAFRF;
		};

		auto parseReportOptions = [](const string &f) {
			if (f == "0") return DMARCReportOptions::ReportOpt_GenerateIfAllFail;
			else if (f == "1") return DMARCReportOptions::ReportOpt_GenerateIfAnyFail;
			else if (f == "d") return DMARCReportOptions::ReportOpt_GenerateAlwaysDKIMFail;
			else if (f == "s") return DMARCReportOptions::ReportOpt_GenerateAlwaysSPFFail;
			else return DMARCReportOptions::ReportOpt_GenerateIfAllFail;
		};

		auto parseTargets = [](const string &raw) {
			vector<DMARCInformTarget> targets = {};
			vector<string> segments = {}; 

			// Parses the list into segments, so we can later make
			//  sense of the parsed segments
			size_t start = 0, end = raw.find_first_of(',');
			for (;;) {
				segments.push_back(raw.substr(start, end - start));
				if (end == string::npos) break;

				start = end + 1;
				end = raw.find_first_of(',', start);
			}

			// Loops over the segments and makes sense of them
			//  such as identifying one as an target address
			for_each(segments.begin(), segments.end(), [&](const string &seg) {
				if (seg.empty()) return;

				// Gets the key value pair from the segment, so we can make use
				//  of the value accordingly
				size_t sep = string::npos;
				if ((sep = seg.find_first_of(':')) == string::npos)
					throw runtime_error(EXCEPT_DEBUG("Coult not parse k/v pair of: '" + seg + '\''));
				
				string key = seg.substr(0, sep), val = seg.substr(++sep);
				transform(key.begin(), key.end(), key.begin(), [](const char c) { return tolower(c); });
				if (*key.begin() == ' ') key.erase(key.begin(), key.begin() + 1);
				if (*(key.end() - 1) == ' ') key.pop_back();
				
				transform(val.begin(), val.end(), val.begin(), [](const char c) { return tolower(c); });
				if (*val.begin() == ' ') val.erase(val.begin(), val.begin() + 1);
				if (*(val.end() - 1) == ' ') val.pop_back();

				// Checks the key, and makes sense of the data, such as using
				//  the address as report target
				if (key == "mailto") {
					targets.push_back(DMARCInformTarget {
						val, DMARCInformType::InformMailto
					});
				}
			});

			return targets;
		};

		for_each(segments.begin(), segments.end(), [&](const string &seg) {
			if (seg.empty()) return;
			
			// Splits the segment into a key / value pair, so we can
			//  later parse the value from it, based on the key
			size_t sep = string::npos;
			if ((sep = seg.find_first_of('=')) == string::npos) 
				throw runtime_error(EXCEPT_DEBUG("Could not parse k/v pair from: '" + seg + '\''));

			string key = seg.substr(0, sep), val = seg.substr(++sep);
			transform(key.begin(), key.end(), key.begin(), [](const char c) { return tolower(c); });
			if (*key.begin() == ' ') key.erase(key.begin(), key.begin() + 1);
			if (*(key.end() - 1) == ' ') key.pop_back();
			
			transform(val.begin(), val.end(), val.begin(), [](const char c) { return tolower(c); });
			if (*val.begin() == ' ') val.erase(val.begin(), val.begin() + 1);
			if (*(val.end() - 1) == ' ') val.pop_back();

			// Checks the key, and makes sense of the value stored inside of it
			if (key == "v") { // Version
				if (val == "dmarc1") this->m_Version = DMARCVersion::DMARC1;
				else this->m_Version = DMARCVersion::DMARC1;
			} else if (key == "p") { // Policy
				this->m_Policy = parsePolicy(val);
			} else if (key == "sp") { // Subdomain policy
				this->m_SubdomainPolicy = parsePolicy(val);
			} else if (key == "pct") { // Filtering percentage
				try {
					this->m_FilteringPercentage = stoi(val);
					if (this->m_FilteringPercentage < 0) this->m_FilteringPercentage = 0;
					else if (this->m_FilteringPercentage > 100) this->m_FilteringPercentage = 100;
				}  catch (...) { this->m_FilteringPercentage = _FSMTP_DMARC_REPORT_FILTER_PERC_DEFAULT; }
			} else if (key == "aspf") { //SPF alignment mode
				this->m_SPFAlignment = parseAlignment(val);
			} else if (key == "adkim") { // Alignment mode for DKIM
				this->m_DKIMAlignment = parseAlignment(val);
			} else if (key == "ruf") { // Forensic reports
				this->m_FeronsicReportTargets = parseTargets(val);
			} else if (key == "rua") { // Aggregate reports
				this->m_FailReportTargets = parseTargets(val);
			} else if (key == "rf") { // Report format
				this->m_ReportFormat = parseFormat(val);
			} else if (key == "ri") { // Report interval
				try {
					this->m_ReportInterval = stol(val);
					if (this->m_ReportInterval < 1000) this->m_ReportInterval = 1000;
				} catch (...) { this->m_ReportInterval = _FSMTP_DMARC_REPORT_INTERVAL_DEFAULT; }
			} else if (key == "fo") { // Failure report options
				this->m_ReportOptions = parseReportOptions(val);
			}
		});

		return *this;
	}

	const char *DMARCRecord::getPolicyString() {
		__dmarcPolicyToString(this->m_Policy);
	}
	
	const char *DMARCRecord::getSubdomainPolicyString() {
		return __dmarcPolicyToString(this->m_SubdomainPolicy);
	}

	const char *DMARCRecord::getDKIMAlignmentString() {
		return __dmarcAlignmentToString(this->m_DKIMAlignment);
	}

	const char *DMARCRecord::getSPFAlignmentString() {
		return __dmarcAlignmentToString(this->m_SPFAlignment);
	}

	const char *DMARCRecord::getReportFormatString() {
		return __dmarcReportFormatToString(this->m_ReportFormat);
	}

	const char *DMARCRecord::getVersionString() {
		return __dmarcVersionToString(this->m_Version);
	}

	const char *DMARCRecord::getReportOptionsString() {
		return __dmarcReportOptionsToString(this->m_ReportOptions);
	}

	DMARCRecord &DMARCRecord::print(Logger &logger) {
		logger << DEBUG;

		auto printTargets = [&](const vector<DMARCInformTarget> &targets) {
			size_t i = 0;
			for_each(targets.begin(), targets.end(), [&](const DMARCInformTarget &t) {
				logger << "\t\t" << i++ << " -> Target { Type: '" << __dmarcInformTypeToString(t.t_Type)
					<< "', Data: '" << t.t_Data << "' }" << ENDL;
			});
		};

		logger << "DMARCRecord {" << ENDL;
		logger << "\tVersion: " << this->getVersionString() << ENDL;
		logger << "\tPolicy: " << this->getPolicyString() << ENDL;
		logger << "\tSubdomain Policy: " << this->getSubdomainPolicyString() << ENDL;
		logger << "\tDKIMAlignment: " << this->getDKIMAlignmentString() << ENDL;
		logger << "\tSPFAlignment: " << this->getSPFAlignmentString() << ENDL;
		logger << "\tReport format: " << this->getReportFormatString() << ENDL;
		logger << "\tFiltering percentage: " << this->m_FilteringPercentage << '%' << ENDL;
		logger << "\tReport interval: " << this->m_ReportInterval << " seconds" << ENDL;
		logger << "\tReport options: " << this->getReportOptionsString() << " seconds" << ENDL;
		logger << ENDL;
		logger << "\tFailure report targets: " << ENDL;
		printTargets(this->m_FailReportTargets);
		logger << "\tForensic report targets: " << ENDL;
		printTargets(this->m_FeronsicReportTargets);
		logger << "}" << ENDL;
	}

	DMARCRecord DMARCRecord::fromDNS(const char *query) {
		DMARCRecord res;

		// Resolves the DNS records, we store them inside an vector
		//  of resolve records
		DNS::Resolver resolver;
		vector<DNS::RR> records = resolver.query(query, ns_t_txt).initParse().getTXTRecords();

		// Loops over the records and attempts to find the DMARC one
		bool found = false;
		all_of(records.begin(), records.end(), [&](const DNS::RR &rr) {
			try {
				res.parse(rr.getData());
				found = true;
				return false;
			} catch (const runtime_error &e) {
				return true;
			} 
		});

		// Checks if we found an valid record, if so return it, else
		//  we throw an runtime error since there is nothing
		if (!found) throw runtime_error(EXCEPT_DEBUG("Could not find valid DMARC record"));
		return res;
	}

	DMARCRecord::~DMARCRecord() noexcept = default;
}