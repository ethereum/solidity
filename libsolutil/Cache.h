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
// SPDX-License-Identifier: GPL-3.0
/** @file Cache.h
 * @date 2024
 *
 * Simple cache.
 */

#pragma once

#include <liblangutil/Exceptions.h>

#include <libsolutil/FixedHash.h>
#include <libsolutil/JSON.h>
#include <libsolutil/Keccak256.h>

#include <boost/functional/hash.hpp>

#include <map>

namespace solidity::util
{

namespace detail
{

template<template<typename, typename> typename TCache ,typename THash, typename TValue>
class CacheBase
{
public:
	typedef THash Hash;
	typedef TValue Value;
	typedef TCache<Hash, Value> Cache;
	typedef std::shared_ptr<Value> Entry;
	typedef std::shared_ptr<Cache> Ptr;

	Entry set(Hash const& hash, Value const& value)
	{
		Entry result;
		if (m_cache.find(hash) == m_cache.end())
		{
			result = std::make_shared<Value>(value);
			auto [_, inserted] = m_cache.emplace(std::make_pair(hash, result));
			solAssert(inserted);
		}
		else
			result = m_cache[hash];
		return result;
	}

	Entry set(Value const& value) { return set(Cache::hash(value), value); }

	std::map<Hash, Entry> const& cache() { return m_cache; }

	typename std::map<Hash, Entry>::iterator get(Hash const& hash) { return m_cache.find(hash); }

	typename std::map<Hash, Entry>::iterator begin() { return m_cache.begin(); }

	typename std::map<Hash, Entry>::iterator end() { return m_cache.end(); }

private:
	std::map<Hash, Entry> m_cache;
};

} // namespace detail

template<typename THash, typename TValue>
class Cache;

template<typename TValue>
class Cache<size_t, TValue>: public detail::CacheBase<Cache, size_t, TValue>
{
public:
	static size_t hash(TValue const& value)
	{
		boost::hash<TValue> hasher;
		return hasher(value);
	}
};

template<typename TValue>
class Cache<h256, TValue>: public detail::CacheBase<Cache, h256, TValue>
{
public:
	static h256 hash(TValue const& value)
	{
		std::stringstream stream;
		stream << value;
		return keccak256(stream.str());
	}
};

template<>
class Cache<size_t, Json>: public detail::CacheBase<Cache, size_t, Json>
{
public:
	static size_t hash(Json const& value)
	{
		boost::hash<std::string> hasher;
		return hasher(value.dump(0));
	}
};

template<>
class Cache<h256, Json>: public detail::CacheBase<Cache, h256, Json>
{
public:
	static h256 hash(Json const& value) { return keccak256(value.dump(0)); }
};

} // namespace solidity::util
