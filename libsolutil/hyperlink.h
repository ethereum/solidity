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

#include <boost/filesystem.hpp>

#include <iostream>
#include <string>

// Required for gethostname()
#if defined(_WIN32)
#include <Winsock2.h>
#else
#include <unistd.h>
#endif

namespace solidity::util
{

struct Hyperlink
{
	bool enabled = false;

	struct Ref { bool enabled; std::string text; };

	Ref operator()(std::string const& _ref) { return Ref{enabled, _ref}; }
};

inline std::ostream& operator<<(std::ostream& _os, Hyperlink::Ref const& _hyperlink)
{
	auto const path = boost::filesystem::path{_hyperlink.text};
	bool const candidate = boost::filesystem::is_regular_file(path) && (&_os == &std::cout || &_os == &std::cerr);

	if (_hyperlink.enabled && candidate)
	{
		static std::string const hostname = []() -> std::string {
			char hostname[80] = {0};
			if (gethostname(hostname, sizeof(hostname)) < 0)
				return std::string{};
			return std::string{hostname};
		}();

		auto const abspath = boost::filesystem::canonical(boost::filesystem::absolute(path));
		auto constexpr OSC8 = "\033]8;;";
		auto constexpr ST = "\033\\";

		_os << OSC8 << "file://" << hostname << abspath.generic_string() << ST;
		_os << _hyperlink.text;
		_os << OSC8 << ST;
	}
	else
		_os << _hyperlink.text;

	return _os;
}

} // end namespace
