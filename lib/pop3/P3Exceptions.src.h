/*
	Copyright [2020] [Luke A.C.A. Rieff]

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language gove;rning permissions and
	limitations under the License.
*/

#ifndef _LIB_POP3_EXCEPTIONS_H
#define _LIB_POP3_EXCEPTIONS_H

#include "../default.h"

namespace FSMTP::POP3 {
  CUSTOM_EXCEPTION(P3OrderException);
  CUSTOM_EXCEPTION(P3SyntaxException);
  CUSTOM_EXCEPTION(P3FatalException);
  CUSTOM_EXCEPTION(P3InvalidCommand);
}

#endif