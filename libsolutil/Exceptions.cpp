// SPDX-License-Identifier: GPL-3.0

#include <libsolutil/Exceptions.h>

using namespace std;
using namespace solidity::util;

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
