// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <map>
#include <set>

/**
 * Data structure that keeps track of values and keys of a mapping.
 */
template <class K, class V>
struct InvertibleMap
{
	std::map<K, V> values;
	// references[x] == {y | values[y] == x}
	std::map<V, std::set<K>> references;

	void set(K _key, V _value)
	{
		if (values.count(_key))
			references[values[_key]].erase(_key);
		values[_key] = _value;
		references[_value].insert(_key);
	}

	void eraseKey(K _key)
	{
		if (values.count(_key))
			references[values[_key]].erase(_key);
		values.erase(_key);
	}

	void eraseValue(V _value)
	{
		if (references.count(_value))
		{
			for (V v: references[_value])
				values.erase(v);
			references.erase(_value);
		}
	}

	void clear()
	{
		values.clear();
		references.clear();
	}
};

template <class T>
struct InvertibleRelation
{
	/// forward[x] contains y <=> backward[y] contains x
	std::map<T, std::set<T>> forward;
	std::map<T, std::set<T>> backward;

	void insert(T _key, T _value)
	{
		forward[_key].insert(_value);
		backward[_value].insert(_key);
	}

	void set(T _key, std::set<T> _values)
	{
		for (T v: forward[_key])
			backward[v].erase(_key);
		for (T v: _values)
			backward[v].insert(_key);
		forward[_key] = std::move(_values);
	}

	void eraseKey(T _key)
	{
		for (auto const& v: forward[_key])
			backward[v].erase(_key);
		forward.erase(_key);
	}
};
