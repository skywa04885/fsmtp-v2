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

#include <sstream>
#include <iostream>

// ============================================
// DEBUG / ERROR stuff
// ============================================

/**
 * Adds the line, file and the message into one string
 *
 * @Param {const char *} file
 * @Param {const std::size_t} line
 * @Param {const std::string &mess}
 * @Return {std::string}
 */
std::string __except_debug(const char *file, 
	const std::size_t line, const std::string &mess);

#ifdef _SMTP_DEBUG
#define DEBUG_ONLY(A) A
#else
#define DEBUG_ONLY(A)
#endif

#ifdef _SMTP_DEBUG
#define DEBUG_PRINT(A, B) A << DEBUG << B << ENDL << CLASSIC
#else
#define DEBUG_PRINT(A, B)
#endif

#ifdef _SMTP_DEBUG
#define EXCEPT_DEBUG(A) __except_debug(__FILE__, __LINE__, A)
#else
#define EXCEPT_DEBUG(A) A
#endif

#define FATAL_ERROR(A) std::cerr << "\033[32m" << __LINE__ << '@' << __FILE__ << "\033[31m" << ": " << A << "\033[0m" << std::endl

// ============================================
// Definitions for quick help
// ============================================

#define BINARY_COMPARE(A, B) (A & B) == B

#ifndef _SMTP_RECEIVE_BUFFER_SIZE
#define _SMTP_RECEIVE_BUFFER_SIZE 32
#endif

#define _PREP_TO_STRING(A) #A
#define PREP_TO_STRING(A) _PREP_TO_STRING(A)

#define _BASH_CHECKMARK "\u2713"
#define _BASH_CROSS "\u2717"
#define _BASH_SUCCESS_MARK "\033[32m[\u2713]:\033[0m "
#define _BASH_FAIL_MARK "\033[31m[\u2717]:\033[0m "
#define _BASH_UNKNOWN_MARK "\033[35m[?]:\033[0m "

typedef enum : uint8_t
{
	ST_SMTP = 0,
	ST_POP3,
	ST_IMAP,
	ST_DNS
} ServerType;

#define assertm(exp, msg) assert(((void)msg, exp))