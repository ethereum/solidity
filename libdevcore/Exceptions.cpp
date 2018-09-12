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

#include <libdevcore/Exceptions.h>

using namespace std;
using namespace dev;

char const* Exception::what() const noexcept
{
	// Return the comment if available.
	if (string const* cmt = comment())
		return cmt->data();

	// Fallback to base what().
	// Boost accepts nullptr, but the C++ standard doesn't
	// and crashes on some platforms.
	return std::exception::what();
}

string Exception::lineInfo() const
{
	char const* const* file = boost::get_error_info<boost::throw_file>(*this);
	int const* line = boost::get_error_info<boost::throw_line>(*this);
	string ret;
	if (file)
		ret += *file;
	ret += ':';
	if (line)
		ret += to_string(*line);
	return ret;
}

string const* Exception::comment() const noexcept
{
	return boost::get_error_info<errinfo_comment>(*this);
}
