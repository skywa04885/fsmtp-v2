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

#include <iostream>
#include <cstdint>
#include <thread>
#include <vector>
#include <mutex>

#include <ncurses.h>

#include "../general/macros.src.h"

#define _NCURSES_BG_COLOR COLOR_BLACK
#define _NCURSES_TEXT_COLOR COLOR_WHITE

#define _NCURSES_CP_FATAL 0
#define _NCURSES_CP_ERROR 1
#define _NCURSES_CP_WARNING 2
#define _NCURSES_CP_INFO 3
#define _NCURSES_CP_DEBUG 4
#define _NCURSES_CP_PARSERS 5

namespace FSMTP
{
	typedef enum : uint8_t
	{
		NDS_STARTING = 0,
		NDS_RUNNING,
		NDS_SHUTDOWN,
		NDS_RESTART
	} NCursesDisplayStatus;

	typedef enum : uint8_t
	{
		NDP_PARSERS = 0,
		NDP_WORKERS,
		NDP_BULLSHIT
	} NCursesDisplayPos;

	typedef enum : uint8_t
	{
		NCL_DEBUG = 0,
		NCL_PARSER,
		NCL_INFO,
		NCL_WARN,
		NCL_ERROR,
		NCL_FATAL
	} NCursesLevel;

	class NCursesDisplay
	{
	public:
		/**
		 * Initializes NCurses
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		static void init(void);
		
		/**
		 * Closes NCurses
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		static void die(void);

		/**
		 * Sets the status of the application
		 *
		 * @Param {const NCursesDisplayStatus} status
		 * @Return {void}
		 */
		static void setStatus(const NCursesDisplayStatus status);

		/**
		 * Sets the thread count
		 *
		 * @Param {const std::size_t} n
		 * @Return {void}
		 */
		static void setThreads(const std::size_t n);

		/**
		 * Sets the emails handled count
		 *
		 * @Param {const std::size_t} n
		 * @Return {void}
		 */
		static void setEmailsHandled(const std::size_t n);

		/**
		 * Sets the email sent count
		 *
		 * @Param {const std::size_t} n
		 * @Return {void}
		 */
		static void setEmailsSent(const std::size_t n);

		/**
		 * Waits for the 'Q' quit press
		 *
		 * @Param {void}
		 * @Return {void}
		 */
		static void listenForQuit(void);

		/**
		 * Prints something to an display
		 *
		 * @Param {const std::string &} raw
		 * @Param {const NCursesDisplayPos} pos
		 * @Param {const NCursesLevel} level
		 * @Param {const char *} prefix
		 */
		static void print(
			const std::string &raw,
			const NCursesDisplayPos pos,
			const NCursesLevel level,
			const char *prefix
		);
	};
}