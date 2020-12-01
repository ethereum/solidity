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

#pragma once

#include <unordered_map>

/**
 * Data structure that keeps track of values and keys of a mapping.
 */
template <class K, class V>
struct InvertibleMap
{
	std::unordered_map<K, V> values;

	void set(K _key, V _value)
	{
		values[_key] = _value;
	}

	std::optional<V> fetch(K _key)
	{
		auto it = values.find(_key);
		if (it == values.end())
			return std::nullopt;
		else
			return it->second;
	}

	void eraseKey(K _key)
	{
		values.erase(_key);
	}

	void eraseValue(V _value)
	{
		auto it = values.begin();
		while (it != values.end())
		{
			if (it->second == _value)
				it = values.erase(it);
			else
				++it;
		}
	}

	void clear()
	{
		values.clear();
	}
};
