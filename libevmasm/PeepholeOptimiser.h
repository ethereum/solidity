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
 * @file PeepholeOptimiser.h
 * Performs local optimising code changes to assembly.
 */
#pragma once

#include <vector>
#include <cstddef>
#include <iterator>

namespace dev
{
namespace eth
{
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;

class PeepholeOptimisationMethod
{
public:
	virtual size_t windowSize() const;
	virtual bool apply(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems> _out);
};

class PeepholeOptimiser
{
public:
	explicit PeepholeOptimiser(AssemblyItems& _items): m_items(_items) {}

	bool optimise();

private:
	AssemblyItems& m_items;
	AssemblyItems m_optimisedItems;
};

}
}
