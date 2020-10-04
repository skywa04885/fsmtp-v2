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

#include "../default.h"
#include "../general/cleanup.src.h"
#include "../general/Logger.src.h"
#include "../general/connections.src.h"
#include "../general/exceptions.src.h"

// The MIMEHeader prototype
namespace FSMTP::MIME {
	struct MIMEHeader;
}

using namespace FSMTP::Cleanup;
using namespace FSMTP::Connections;

namespace FSMTP::Models
{
	class EmailAddress {
	public:
		EmailAddress(const string &e_Name, const string &e_Address);
		EmailAddress(const string &raw);
		explicit EmailAddress();

		void parse(const string &raw);

		string getDomain() const;
		string getUsername() const;
		static vector<EmailAddress> parseAddressList(const string &raw);
		static string addressListToString(const vector<EmailAddress> &addresses);
		string toString() const;

		string e_Name;
		string e_Address;
	};

	typedef enum : uint32_t {
		ET_INCOMMING = 0,
		ET_INCOMMING_SPAM,
		ET_OUTGOING,
		ET_RELAY_OUTGOING,
	} EmailType;

	typedef enum : uint8_t {
		ETE_8BIT = 0,
		ETE_7BIT,
		ETE_BASE64,
		ETE_QUOTED_PRINTABLE,
		ETE_NOT_FUCKING_KNOWN,
		ETE_NOT_FOUND
	} EmailTransferEncoding;

	typedef enum : uint8_t {
		ECT_TEXT_PLAIN = 0,
		ECT_TEXT_HTML,
		ECT_MULTIPART_ALTERNATIVE,
		ECT_MULTIPART_MIXED,
		ECT_NOT_FUCKING_KNOWN,
		ECT_NOT_FOUND,
		ECT_IMAGE_PNG,
		ECT_IMAGE_JPG,
		ECT_IMAGE_GIF,
		ECT_FILE_PDF,
		ECT_FILE_OTHER
	} EmailContentType;

	EmailContentType stringToEmailContentType(const string &raw);
	const char *contentTypeToString(const EmailContentType type);
	const char *contentTransferEncodingToString(const EmailTransferEncoding enc);
	EmailTransferEncoding stringToEmailTransferEncoding(const string &raw);

	typedef struct {
		string e_Content;
		EmailContentType e_Type;
		vector<MIME::MIMEHeader> e_Headers;
		int32_t e_Index;
		EmailTransferEncoding e_TransferEncoding;
	} EmailBodySection;

	class FullEmail {
	public:
    FullEmail();

    static int64_t getBucket();
    static CassUuid generateMessageUUID();
    static void print(FullEmail &email, Logger &logger);

    	EmailAddress e_TransportFrom;
		vector<EmailAddress> e_TransportTo;
		string e_Subject;
		string e_MessageID;
		vector<EmailBodySection> e_BodySections;
		vector<EmailAddress> e_From;
		vector<EmailAddress> e_To;
		vector<MIME::MIMEHeader> e_Headers;
		size_t e_Date;
	};
}
