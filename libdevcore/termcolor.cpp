/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libdevcore/termcolor.h>

#include <iostream>
#include <ostream>
#include <utility>

#if defined(__unix__)
#include <unistd.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace std;

namespace termcolor {

namespace internal {

static int const enabled_index = std::ios_base::xalloc();

void setupTerminal()
{
#if defined(_WIN32) && defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
	// Set output mode to handle virtual terminal (ANSI escape sequences)
	// ignore any error, as this is just a "nice-to-have"
	// only windows needs to be taken care of, as other platforms (Linux/OSX) support them natively.
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return;

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
		return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
		return;
#endif
}

}  // namespace internal

bool is_terminal(ostream const& os)
{
	if (&os == &cout)
#if defined(__unix__)
		return isatty(STDOUT_FILENO);
#else
		return true;
#endif

	if (&os == &cerr)
#if defined(__unix__)
		return isatty(STDERR_FILENO);
#else
		return true;
#endif

	return false;
}

bool is_enabled(ostream const& os)
{
	return const_cast<ostream&>(os).iword(internal::enabled_index) != 0;
}

ostream& enable(ostream& os)
{
	if (!is_terminal(os))
		internal::setupTerminal();

	os.iword(internal::enabled_index) = 1;
	return os;
}

ostream& disable(ostream& os)
{
	os.iword(internal::enabled_index) = 0;
	return os;
}

void enable_if(ostream& os, bool test)
{
	if (test)
		os << enable;
}

template <typename T>
inline internal::CodeSeq colorize(T code)
{
	return {static_cast<unsigned>(code)};
}

// {{{ controls
ostream& reset(ostream& os)
{
	return os << colorize(control::reset);
}

ostream& bold(ostream& os)
{
	return os << colorize(control::bold);
}

ostream& underline(ostream& os)
{
	return os << colorize(control::underline);
}
// }}}

// {{{ foreground colors
internal::CodeSeq text(uint8_t code)
{
	// "\033[38;5;<FG COLOR>m"
	return {38, 5, code};
}

internal::CodeSeq text(color c)
{
	return {30 + static_cast<unsigned>(c)};
}

ostream& black(ostream& os)
{
	return os << text(color::black);
}

ostream& red(ostream& os)
{
	return os << text(color::red);
}

ostream& green(ostream& os)
{
	return os << text(color::green);
}

ostream& yellow(ostream& os)
{
	return os << text(color::yellow);
}

ostream& blue(ostream& os)
{
	return os << text(color::blue);
}

ostream& magenta(ostream& os)
{
	return os << text(color::magenta);
}

ostream& cyan(ostream& os)
{
	return os << text(color::cyan);
}

ostream& white(ostream& os)
{
	return os << text(color::white);
}
// }}}

// {{{ background colors
internal::CodeSeq background(uint8_t code)
{
	return {48, 5, code};
}

internal::CodeSeq background(color c)
{
	return {40 + static_cast<unsigned>(c)};
}

ostream& on_black(ostream& os)
{
	return os << background(color::black);
}

ostream& on_red(ostream& os)
{
	return os << background(color::red);
}

ostream& on_green(ostream& os)
{
	return os << background(color::green);
}

ostream& on_yellow(ostream& os)
{
	return os << background(color::yellow);
}

ostream& on_blue(ostream& os)
{
	return os << background(color::blue);
}

ostream& on_magenta(ostream& os)
{
	return os << background(color::magenta);
}

ostream& on_cyan(ostream& os)
{
	return os << background(color::cyan);
}

ostream& on_white(ostream& os)
{
	return os << background(color::white);
}
// }}}

} // namespace termcolor

ostream& operator<<(ostream& os, termcolor::internal::CodeSeq const& codes)
{
	if (termcolor::is_enabled(os) && !codes.empty())
	{
		os << '\033' << '[';
		for (size_t i = 0; i < codes.size(); ++i)
			if (i)
				os << ';' << codes[i];
			else
				os << codes[i];
		os << 'm';
	}
	return os;
}
