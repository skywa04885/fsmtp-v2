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

#ifndef _LIB_XFANNST_FLAGS_H
#define _LIB_XFANNST_FLAGS_H

#include "../default.h"
#include "../general/Logger.src.h"

#define _FSMTP_XFANNST_FLAG_STORAGE_NOSTORE _BV(1)

#define _FSMTP_XFANNST_FLAG_MAILER_NOERROR _BV(1)

namespace FSMTP::XFannst {
    string __fannstStorageFlagsToString(uint8_t flags);
    string __fannstMailerFlagsToString(uint8_t flags);

    class XFannstFlags {
    public:
        XFannstFlags();

        XFannstFlags &parse(const string &raw);

        bool getNoStore();
        bool getNoError();

        string getMailerFlagsString();
        string getStorageFlagsString();

        XFannstFlags &print(Logger &logger);

        ~XFannstFlags();
    private:
        uint8_t m_MailerFlags;
        uint8_t m_StorageFlags;
    };
}

#endif
