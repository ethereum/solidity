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

#pragma once

#include <cstdint>
#include <ios>
#include <iosfwd>
#include <vector>

/**
 * Terminal color API.
 *
 * Color output is disabled by default. Enable it in your main() in a way like:
 * <code>
 *     termcolor::enable_if(cout, termcolor::is_terminal(cout) && !NO_COLOR);
 *     termcolor::enable_if(cerr, termcolor::is_terminal(cerr) && !NO_COLOR);
 * </code>
 *
 * Then make use like that:
 * <code>
 *   cout << "Hello " << termcolor::red << "User" << termcolor::reset << '!' << endl;
 * </code>
 */

namespace termcolor
{

namespace internal
{
	using CodeSeq = std::vector<unsigned>;
}

enum class control : unsigned
{
	reset = 0,
	bold = 1,
	underline = 4,
};

enum class color : unsigned
{
	black = 0,
	red,
	green,
	yellow,
	blue,
	magenta,
	cyan,
	white
};

// global control
bool is_terminal(std::ostream const& os);
bool is_enabled(std::ostream const& os);
void enable_if(std::ostream& os, bool test);
std::ostream& enable(std::ostream& os);
std::ostream& disable(std::ostream& os);

// color stream settings
std::ostream& reset(std::ostream& os);
std::ostream& bold(std::ostream& os);
std::ostream& underline(std::ostream& os);

// foreground colors
internal::CodeSeq text(uint8_t code);
internal::CodeSeq text(color c);
std::ostream& black(std::ostream& os);
std::ostream& red(std::ostream& os);
std::ostream& green(std::ostream& os);
std::ostream& yellow(std::ostream& os);
std::ostream& blue(std::ostream& os);
std::ostream& magenta(std::ostream& os);
std::ostream& cyan(std::ostream& os);
std::ostream& white(std::ostream& os);

// background colors
internal::CodeSeq background(uint8_t code);
internal::CodeSeq background(color c);
std::ostream& on_black(std::ostream& os);
std::ostream& on_red(std::ostream& os);
std::ostream& on_green(std::ostream& os);
std::ostream& on_yellow(std::ostream& os);
std::ostream& on_blue(std::ostream& os);
std::ostream& on_magenta(std::ostream& os);
std::ostream& on_cyan(std::ostream& os);
std::ostream& on_white(std::ostream& os);

}  // namespace termcolor

std::ostream& operator<<(std::ostream& os, termcolor::internal::CodeSeq const& codes);
