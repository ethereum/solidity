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

#include <map>

// Contains polyfills of STL functions and algorithms that will become available in C++20.
namespace solidity::cxx20
{

// Taken from https://en.cppreference.com/w/cpp/container/map/erase_if.
template<class Key, class T, class Compare, class Alloc, class Pred>
typename std::map<Key, T, Compare, Alloc>::size_type erase_if(std::map<Key,T,Compare,Alloc>& _c, Pred _pred)
{
	auto old_size = _c.size();
	for (auto i = _c.begin(), last = _c.end(); i != last;)
		if (_pred(*i))
			i = _c.erase(i);
		else
			++i;
	return old_size - _c.size();
}

// Taken from https://en.cppreference.com/w/cpp/container/unordered_map/erase_if.
template<class Key, class T, class Hash, class KeyEqual, class Alloc, class Pred>
typename std::unordered_map<Key, T, Hash, KeyEqual, Alloc>::size_type
erase_if(std::unordered_map<Key, T, Hash, KeyEqual, Alloc>& _c, Pred _pred)
{
	auto old_size = _c.size();
	for (auto i = _c.begin(), last = _c.end(); i != last;)
		if (_pred(*i))
			i = _c.erase(i);
		else
			++i;
	return old_size - _c.size();
}

}
