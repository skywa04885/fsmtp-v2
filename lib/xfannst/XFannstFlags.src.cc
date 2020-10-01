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

#include "XFannstFlags.src.h"

namespace FSMTP::XFannst {
    string __fannstStorageFlagsToString(uint8_t flags) {
        string result;

        if (BINARY_COMPARE(flags, _FSMTP_XFANNST_FLAG_STORAGE_NOSTORE))
            result += "NOSTORE/";

        if (result.size() > 0) result.pop_back();
        return result;
    }

    string __fannstMailerFlagsToString(uint8_t flags) {
        string result;

        if (BINARY_COMPARE(flags, _FSMTP_XFANNST_FLAG_MAILER_NOERROR))
            result += "NOERROR/";

        if (result.size() > 0) result.pop_back();
        return result;
    }

    XFannstFlags::XFannstFlags(): m_MailerFlags(0x0), m_StorageFlags(0x0) {}

    XFannstFlags &XFannstFlags::parse(const string &raw) {
        // ================================
        // Splits the header into segments
        // ================================

        vector<string> segments = {};

        // Loops over the parts in the header, and splits
        //  it by a semicolon
        size_t start = 0, end = raw.find_first_of(';');
        for (;;) {
            segments.push_back(raw.substr(start, end - start));
            if (end == string::npos) break;

            start = end + 1;
            end = raw.find_first_of(';', start);
        }

        // ================================
        // Makes sense of the segments
        // ================================

        auto parseDB = [&](const string &val) {
            start = 0, end = val.find_first_of(':');
            for (;;) {
                string seg = val.substr(start, end - start);
                if (seg == "nstore") this->m_StorageFlags |= _FSMTP_XFANNST_FLAG_STORAGE_NOSTORE;

                if (end == string::npos) break;

                start = end + 1;
                end = val.find_first_of(':', start);
            }
        };

        auto parseMailer = [&](const string &val) {
            start = 0, end = val.find_first_of(':');
            for (;;) {
                string seg = val.substr(start, end - start);
                if (seg == "nerror") this->m_MailerFlags |= _FSMTP_XFANNST_FLAG_MAILER_NOERROR;

                if (end == string::npos) break;

                start = end + 1;
                end = val.find_first_of(':', start);
            }
        };

        // Loops over the segments, and splits them into key/value
        //  pairs, which we will parse
        for_each(segments.begin(), segments.end(), [&](const string &seg) {           
            size_t sep = seg.find_first_of('=');
            if (sep == string::npos)
                throw runtime_error(EXCEPT_DEBUG("Could not parse k/v pair from header: '" + seg + '\''));

            string key = seg.substr(0, sep), val = seg.substr(++sep);

            // Transforms the key and value to lowercase, after which we remove the extra
            //  whitespace from them ( if there )
            transform(key.begin(), key.end(), key.begin(), [](const char c) { return tolower(c); });
            if(*key.begin() == ' ') key.erase(key.begin(), key.begin() + 1);
            if (*(key.end() - 1) == ' ') key.pop_back();

            transform(val.begin(), val.end(), val.begin(), [](const char c) { return tolower(c); });
            if(*val.begin() == ' ') val.erase(val.begin(), val.begin() + 1);
            if (*(val.end() - 1) == ' ') val.pop_back();


            // Checks the key, to check how we should treat the value
            //  if we do not have anything for it we just ignore it
            if (key == "mailer") parseMailer(val);
            else if (key == "db") parseDB(val);
        });

        return *this;
    }

    bool XFannstFlags::getNoStore() {
        return BINARY_COMPARE(this->m_StorageFlags, _FSMTP_XFANNST_FLAG_STORAGE_NOSTORE);
    }
    
    bool XFannstFlags::getNoError() {
        return BINARY_COMPARE(this->m_MailerFlags, _FSMTP_XFANNST_FLAG_MAILER_NOERROR);
    }

    string XFannstFlags::getMailerFlagsString() {
        return __fannstMailerFlagsToString(this->m_MailerFlags);
    }
    
    string XFannstFlags::getStorageFlagsString() {
        return __fannstStorageFlagsToString(this->m_StorageFlags);
    }

    XFannstFlags &XFannstFlags::print(Logger &logger) {
        logger << DEBUG;

        logger << "XFannstFlags {" << ENDL;
        logger << "\tBinary mailer flags: " << bitset<8>(this->m_MailerFlags) << ENDL;
        logger << "\tBinary storage flags: " << bitset<8>(this->m_StorageFlags) << ENDL;
        logger << "\tStorage flags: " << this->getStorageFlagsString() << ENDL;
        logger << "\tMailer flags: " << this->getMailerFlagsString() << ENDL;
        logger << '}' << ENDL;

        logger << CLASSIC;
        return *this;
    }

    XFannstFlags::~XFannstFlags() = default;
}
