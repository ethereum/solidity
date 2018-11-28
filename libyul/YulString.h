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

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

namespace dev
{
namespace yul
{

/// Repository for YulStrings.
/// Owns the string data for all YulStrings, which can be referenced by a Handle.
/// A Handle consists of an ID (that depends on the insertion order of YulStrings and is potentially
/// non-deterministic) and a deterministic string hash.
class YulStringRepository: boost::noncopyable
{
public:
	struct Handle
	{
		size_t id;
		std::uint64_t hash;
	};
	YulStringRepository():
        m_strings{std::make_shared<std::pair<std::string, size_t>>()},
		m_hashToID{std::make_pair(emptyHash(), 0)}
	{}
	static YulStringRepository& instance()
	{
		static YulStringRepository inst;
		return inst;
	}
	Handle stringToHandle(std::string const& _string, const size_t _suffix)
	{
		if (_string.empty())
			return { 0, emptyHash() };
		std::uint64_t h = hash(_string, _suffix);
		auto range = m_hashToID.equal_range(h);
		for (auto it = range.first; it != range.second; ++it)
			if (m_strings[it->second]->first == _string && m_strings[it->second]->second == _suffix)
                return Handle{it->second, h};
		m_strings.emplace_back(std::make_shared<std::pair<std::string, size_t>>(std::make_pair(_string, _suffix)));
        size_t id = m_strings.size() - 1;
		m_hashToID.emplace_hint(range.second, std::make_pair(h, id));
		return Handle{id, h};
	}
	std::string const idToString(size_t _id) const	{ 
        return m_strings[_id]->second > 0 ? m_strings[_id]->first + "_" + std::to_string(m_strings[_id]->second)
                                          : m_strings[_id]->first;
    }
	std::string const idToPrefix(size_t _id) const	{ 
        return m_strings[_id]->first;
    }


	static std::uint64_t hash(std::string const& v, const size_t s)
	{
		// FNV hash - can be replaced by a better one, e.g. xxhash64
		std::uint64_t hash = emptyHash();
		for (auto c: v)
		{
			hash *= 1099511628211u;
			hash ^= c;
		}

        unsigned char * byte = new unsigned char[8];
        for (int i = 0; i < 8; i++)
        {
            byte[i] = (s >> (56 - (8 * i)) & 0xFF);
        }

		return hash;
	}
	static constexpr std::uint64_t emptyHash() { return 14695981039346656037u; }
private:
    std::vector<std::shared_ptr<std::pair<std::string, size_t>>> m_strings;
	std::unordered_multimap<std::uint64_t, size_t> m_hashToID;
};

/// Wrapper around handles into the YulString repository.
/// Equality of two YulStrings is determined by comparing their ID.
/// The <-operator depends on the string hash and is not consistent
/// with string comparisons (however, it is still deterministic).
class YulString
{
public:
	YulString() = default;
	explicit YulString(std::string const& _s, const size_t suffix = 0): m_handle(YulStringRepository::instance().stringToHandle(_s, suffix)) {}
	YulString(YulString const&) = default;
	YulString(YulString&&) = default;
	YulString& operator=(YulString const&) = default;
	YulString& operator=(YulString&&) = default;

	/// This is not consistent with the string <-operator!
	/// First compares the string hashes. If they are equal
	/// it checks for identical IDs (only identical strings have
	/// identical IDs and identical strings do not compare as "less").
	/// If the hashes are identical and the strings are distinct, it
	/// falls back to string comparison.
	bool operator<(YulString const& _other) const
	{
		if (m_handle.hash < _other.m_handle.hash) return true;
		if (_other.m_handle.hash < m_handle.hash) return false;
		if (m_handle.id == _other.m_handle.id) return false;
		return str() < _other.str();
	}
	/// Equality is determined based on the string ID.
	bool operator==(YulString const& _other) const { return m_handle.id == _other.m_handle.id; }
	bool operator!=(YulString const& _other) const { return m_handle.id != _other.m_handle.id; }

	bool empty() const { return m_handle.id == 0; }
	std::string const str() const
	{
		return YulStringRepository::instance().idToString(m_handle.id);
	}
    std::string const prefix() const 
    {
		return YulStringRepository::instance().idToPrefix(m_handle.id);
    }
    size_t id() const { return m_handle.id; } 

private:
	/// Handle of the string. Assumes that the empty string has ID zero.
	YulStringRepository::Handle m_handle{ 0, YulStringRepository::emptyHash() };
};

}
}
