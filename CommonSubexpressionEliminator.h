/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file CommonSubexpressionEliminator.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Optimizer step for common subexpression elimination and stack reorganisation.
 */

#pragma once

#include <vector>

namespace dev
{
namespace eth
{

class AssemblyItem;

/**
 * Optimizer step that performs common subexpression elimination and stack reorginasation,
 * i.e. it tries to infer equality among expressions and compute the values of two expressions
 * known to be equal only once.
 */
class CommonSubexpressionEliminator
{
public:
	/// Feeds AssemblyItems into the eliminator and @returns the iterator pointing at the first
	/// item that must be fed into a new instance of the eliminator.
	template <class _AssemblyItemIterator>
	_AssemblyItemIterator feedItems(_AssemblyItemIterator _iterator, _AssemblyItemIterator _end);

	/// @returns the resulting items after optimization.
	std::vector<AssemblyItem> getOptimizedItems() const;

private:
	/// @returns true if the given items starts a new basic block
	bool breaksBasicBlock(AssemblyItem const& _item);
	/// Feeds the item into the system for analysis.
	void feedItem(AssemblyItem const& _item);
};

template <class _AssemblyItemIterator>
_AssemblyItemIterator CommonSubexpressionEliminator::feedItems(
	_AssemblyItemIterator _iterator,
	_AssemblyItemIterator _end
)
{
	while (_iterator != _end && !breaksBasicBlock(*_iterator))
		feedItem(*_iterator);
	return _iterator;
}

}
}
