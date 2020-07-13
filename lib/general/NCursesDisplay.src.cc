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

#include "NCursesDisplay.src.h"

namespace FSMTP
{
	static WINDOW *_statusWin = nullptr;
	static WINDOW *_generalWin = nullptr;

	static int32_t _generalLine = 0;

	static int32_t winWidth;
	static int32_t winHeight;

	static int32_t winTerminalHeight;
	static int32_t winTerminalWidth;

	static std::mutex cursedMutex;

	/**
	 * Initializes NCurses
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void NCursesDisplay::init(void)
	{
		// Inits the screen and gets the
		// - width and height
		initscr();
		start_color();
		getmaxyx(stdscr, winHeight, winWidth);

		// Calculates the sub window with
		// - and height
		winTerminalHeight = winHeight;
		winTerminalWidth = winWidth - 40;

		// ===================================
		// Initializes the ncurses displays
		//
		// These are: ["parsers", ["errors",
		// "status", "general"]
		// ===================================

		// Creates the windows
		_statusWin = newwin(winTerminalHeight, 39, 0, 0);
		_generalWin = newwin(winTerminalHeight, winTerminalWidth, 0, 40);
		
		scrollok(_generalWin, true);
		refresh();

		// Adds the boxes
		box(_statusWin, 0, 0);

		// ===================================
		// Initializes the color pairs
		//
		// Nothing interesting
		// ===================================

		init_pair(_NCURSES_CP_FATAL, COLOR_RED, _NCURSES_BG_COLOR);
		init_pair(_NCURSES_CP_ERROR, COLOR_BLUE, _NCURSES_BG_COLOR);
		init_pair(_NCURSES_CP_WARNING, COLOR_YELLOW, _NCURSES_BG_COLOR);
		init_pair(_NCURSES_CP_INFO, COLOR_GREEN, _NCURSES_BG_COLOR);
		init_pair(_NCURSES_CP_DEBUG, COLOR_CYAN, _NCURSES_BG_COLOR);
		init_pair(_NCURSES_CP_PARSERS, COLOR_MAGENTA, _NCURSES_BG_COLOR);

		// ===================================
		// Draws the default status stuff
		//
		// Nothing interesting
		// ===================================

		wmove(_statusWin, 1, 1);
		wprintw(_statusWin, "NodeID: ");
		wprintw(_statusWin, _SMTP_SERVICE_NODE_NAME);

		wmove(_statusWin, 2, 1);
		wprintw(_statusWin, "ServerDomain: ");
		wprintw(_statusWin, _SMTP_SERVICE_DOMAIN);

		wmove(_statusWin, 3, 1);
		wprintw(_statusWin, "SSL_Cert: ");
		wprintw(_statusWin, _SMTP_SSL_CERT_PATH);

		wmove(_statusWin, 4, 1);
		wprintw(_statusWin, "SSL_Key: ");
		wprintw(_statusWin, _SMTP_SSL_KEY_PATH);

		wmove(_statusWin, 5, 1);
		wprintw(_statusWin, "Redis_IP: ");
		wprintw(_statusWin, _REDIS_CONTACT_POINTS);
		wprintw(_statusWin, ":");
		wprintw(_statusWin, PREP_TO_STRING(_REDIS_PORT));

		wmove(_statusWin, 6, 1);
		wprintw(_statusWin, "CassHosts: ");
		wprintw(_statusWin, _CASSANDRA_DATABASE_CONTACT_POINTS);
		wrefresh(_statusWin);

		// Sets the dynamic values
		NCursesDisplay::setThreads(0);
		NCursesDisplay::setEmailsHandled(0);
		NCursesDisplay::setEmailsSent(0);
	}

	/**
	 * Sets the thread count
	 *
	 * @Param {const std::size_t} n
	 * @Return {void}
	 */
	void NCursesDisplay::setThreads(const std::size_t n)
	{
		cursedMutex.lock();
		wmove(_statusWin, 8, 1);
		wprintw(_statusWin, "Threads (Recv): ");
		wprintw(_statusWin, "%d", n);
		wmove(_statusWin, 0, 0);
		wrefresh(_statusWin);
		cursedMutex.unlock();
	}

	/**
	 * Sets the emails handled count
	 *
	 * @Param {const std::size_t} n
	 * @Return {void}
	 */
	void NCursesDisplay::setEmailsHandled(const std::size_t n)
	{
		cursedMutex.lock();
		wmove(_statusWin, 9, 1);
		wprintw(_statusWin, "EmailsRecv: ");
		wprintw(_statusWin, "%d", n);
		wmove(_statusWin, 0, 0);
		wrefresh(_statusWin);
		cursedMutex.unlock();
	}

	/**
	 * Sets the email sent count
	 *
	 * @Param {const std::size_t} n
	 * @Return {void}
	 */
	void NCursesDisplay::setEmailsSent(const std::size_t n)
	{
		cursedMutex.lock();
		wmove(_statusWin, 10, 1);
		wprintw(_statusWin, "EmailSent: ");
		wprintw(_statusWin, "%d", n);
		wmove(_statusWin, 0, 0);
		wrefresh(_statusWin);
		cursedMutex.unlock();
	}

	/**
	 * Closes NCurses
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void NCursesDisplay::die(void)
	{
		// Stops ncurses and deallocates memory
		endwin();
	}

	/**
	 * Sets the status of the application
	 *
	 * @Param {const NCursesDisplayStatus} status
	 * @Return {void}
	 */
	void NCursesDisplay::setStatus(const NCursesDisplayStatus status)
	{
		cursedMutex.lock();
		wmove(_statusWin, 12, 1);
		wclrtoeol(_statusWin);
		box(_statusWin, 0, 0);
		wprintw(_statusWin, "Status: ");
		wattron(_statusWin, A_REVERSE);
		wattron(_statusWin, A_BLINK);
		switch (status)
		{
			case NCursesDisplayStatus::NDS_STARTING:
			{
				wprintw(_statusWin, "OPSTARTEN");
				break;
			}
			case NCursesDisplayStatus::NDS_RUNNING:
			{
				wprintw(_statusWin, "ACTIEF");
				break;
			}
			case NCursesDisplayStatus::NDS_SHUTDOWN:
			{
				wprintw(_statusWin, "AFSLUITEN");
				break;
			}
			case NCursesDisplayStatus::NDS_RESTART:
			{
				wprintw(_statusWin, "HERSTARTEN");
				break;
			}
		}
		wattroff(_statusWin, A_BLINK);
		wattroff(_statusWin, A_REVERSE);
		wmove(_statusWin, 0, 0);
		wrefresh(_statusWin);
		cursedMutex.unlock();
	}

	/**
	 * Prints something to an display
	 *
	 * @Param {const std::string &} raw
	 * @Param {const NCursesDisplayPos} pos
	 * @Param {const NCursesLevel} level
	 * @Param {const char *} prefix
	 */
	void NCursesDisplay::print(
		const std::string &raw,
		const NCursesDisplayPos pos,
		const NCursesLevel level,
		const char *prefix
	)
	{
		cursedMutex.lock();

		// Gets the correct color attribute,
		// - and stores it in the attr
		int attr;
		switch (level)
		{
			case NCursesLevel::NCL_ERROR:
			{
				attr = _NCURSES_CP_ERROR;
				break;
			}
			case NCursesLevel::NCL_FATAL:
			{
				attr = _NCURSES_CP_FATAL;
				break;
			}
			case NCursesLevel::NCL_DEBUG:
			{
				attr = _NCURSES_CP_DEBUG;
				break;
			}
			case NCursesLevel::NCL_PARSER:
			{
				attr = _NCURSES_CP_PARSERS;
				break;
			}
			case NCursesLevel::NCL_WARN:
			{
				attr = _NCURSES_CP_WARNING;
				break;
			}
			case NCursesLevel::NCL_INFO:
			{
				attr = _NCURSES_CP_INFO;
				break;
			}
		}

		wscrl(_generalWin, -1);

		// Prints the message
		wprintw(_generalWin, "%d->", std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count());
		wattron(_generalWin, COLOR_PAIR(attr));
		wattron(_generalWin, A_BOLD);
		switch (level)
		{
			case NCursesLevel::NCL_ERROR:
			{
				wprintw(_generalWin, "[error%s]: ", prefix);
				break;
			}
			case NCursesLevel::NCL_FATAL:
			{
				wprintw(_generalWin, "[fatal@%s]: ", prefix);
				break;
			}
			case NCursesLevel::NCL_DEBUG:
			{
				wprintw(_generalWin, "[debug@%s]: ", prefix);
				break;
			}
			case NCursesLevel::NCL_PARSER:
			{
				wprintw(_generalWin, "[parser@%s]: ", prefix);
				break;
			}
			case NCursesLevel::NCL_WARN:
			{
				wprintw(_generalWin, "[warn@%s]: ", prefix);
				break;
			}
			case NCursesLevel::NCL_INFO:
			{
				wprintw(_generalWin, "[info@%s]: ", prefix);
				break;
			}
		}
		wattroff(_generalWin, COLOR_PAIR(attr));
		wattroff(_generalWin, A_BOLD);
		wprintw(_generalWin, "%s", raw.c_str());

		wmove(_generalWin, 0, 0);
		wrefresh(_generalWin);

		cursedMutex.unlock();
	}

	/**
	 * Waits for the 'Q' quit press
	 *
	 * @Param {void}
	 * @Return {void}
	 */
	void NCursesDisplay::listenForQuit(void)
	{
		while (mvwgetch(_generalWin, 0, 0) != 'q') continue;
	}
}