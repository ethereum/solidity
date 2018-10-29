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
/**
 * String abstraction that avoids copies.
 */

#pragma once

#include <boost/noncopyable.hpp>

#include <map>
#include <memory>
#include <vector>
#include <string>

namespace dev
{
namespace yul
{

class YulStringRepository: boost::noncopyable
{
public:
	YulStringRepository(): m_strings{std::make_shared<std::string>()}
	{
		m_ids[std::string{}] = 0;
	}
	static YulStringRepository& instance()
	{
		static YulStringRepository inst;
		return inst;
	}
	size_t stringToId(std::string const& _string)
	{
		if (_string.empty())
			return 0;
		size_t& id = m_ids[_string];
		if (id == 0)
		{
			m_strings.emplace_back(std::make_shared<std::string>(_string));
			id = m_strings.size() - 1;
		}
		return id;
	}
	std::string const& idToString(size_t _id) const
	{
		return *m_strings.at(_id);
	}

private:
	std::vector<std::shared_ptr<std::string>> m_strings;
	std::map<std::string, size_t> m_ids;
};

class YulString
{
public:
	YulString() = default;
	explicit YulString(std::string const& _s): m_id(YulStringRepository::instance().stringToId(_s)) {}
	YulString(YulString const&) = default;
	YulString(YulString&&) = default;
	YulString& operator=(YulString const&) = default;
	YulString& operator=(YulString&&) = default;

	/// This is not consistent with the string <-operator!
	bool operator<(YulString const& _other) const { return m_id < _other.m_id; }
	bool operator==(YulString const& _other) const { return m_id == _other.m_id; }
	bool operator!=(YulString const& _other) const { return m_id != _other.m_id; }

	bool empty() const { return m_id == 0; }
	std::string const& str() const
	{
		return YulStringRepository::instance().idToString(m_id);
	}

private:
	/// ID of the string. Assumes that the empty string has ID zero.
	size_t m_id = 0;
};

}
}
