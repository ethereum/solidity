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

namespace dev
{
namespace solidity
{
namespace test
{

class FormattedPrinter
{
public:
	FormattedPrinter(bool _enabled) : m_enabled(_enabled)
	{
	}

	static constexpr char const* RESET = "\033[0m";
	static constexpr char const* RED  = "\033[1;31m";
	static constexpr char const* GREEN = "\033[1;32m";
	static constexpr char const* YELLOW = "\033[1;33m";
	static constexpr char const* CYAN = "\033[1;36m";
	static constexpr char const* BOLD = "\033[1m";
	static constexpr char const* INVERSE = "\033[7m";

	void enableFormatting()
	{
		m_enabled = true;
	}
	void disableFormatting()
	{
		m_enabled = false;
	}

	class Scope
	{
	public:
		Scope(std::ostream& _stream, bool const _enabled) : m_stream(_stream), m_enabled(_enabled)
		{
		}
		~Scope()
		{
			try
			{
				if (m_enabled) m_stream << RESET;
			}
			catch(...)
			{
			}
		}
		template<typename T>
		std::ostream& operator<<(T&& t)
		{
			return m_stream << t;
		}
	private:
		std::ostream& m_stream;
		bool const m_enabled;
	};
	Scope format(std::ostream& _stream, std::vector<const char*> const& _formatting) const
	{
		if (m_enabled)
			for (auto &format : _formatting)
				_stream << format;
		return Scope(_stream, m_enabled);
	}
private:
	bool m_enabled;
};


}
}
}
