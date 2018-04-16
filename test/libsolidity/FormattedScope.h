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

#include <boost/noncopyable.hpp>

#include <ostream>
#include <vector>

namespace dev
{
namespace solidity
{
namespace test
{

namespace formatting
{

static constexpr char const* RESET = "\033[0m";
static constexpr char const* RED  = "\033[1;31m";
static constexpr char const* GREEN = "\033[1;32m";
static constexpr char const* YELLOW = "\033[1;33m";
static constexpr char const* CYAN = "\033[1;36m";
static constexpr char const* BOLD = "\033[1m";
static constexpr char const* RED_BACKGROUND  = "\033[48;5;160m";
static constexpr char const* ORANGE_BACKGROUND  = "\033[48;5;166m";
static constexpr char const* INVERSE = "\033[7m";

}

class FormattedScope: boost::noncopyable
{
public:
	/// @arg _formatting List of formatting strings (e.g. colors) defined in the formatting namespace.
	FormattedScope(std::ostream& _stream, bool const _enabled, std::vector<char const*> const& _formatting):
		m_stream(_stream), m_enabled(_enabled)
	{
		if (m_enabled)
			for (auto const& format: _formatting)
				m_stream << format;
	}
	~FormattedScope() { if (m_enabled) m_stream << formatting::RESET; }
	template<typename T>
	std::ostream& operator<<(T&& _t) { return m_stream << std::forward<T>(_t); }
private:
	std::ostream& m_stream;
	bool m_enabled;
};

}
}
}
