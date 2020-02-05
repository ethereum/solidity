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
 * Contains an abstract base class representing a selection of pairs of elements from a collection
 * and its concrete implementations.
 */

#pragma once

#include <tuple>
#include <vector>

namespace solidity::phaser
{

/**
 * Abstract base class for selections of pairs elements from a collection.
 *
 * An instance of this class represents a specific method of selecting a set of pairs of elements
 * from containers of arbitrary sizes. The selected pairs always point at a subset of the elements
 * from the container but may indicate the same element more than once. The pairs themselves can
 * repeat too. The selection may or may not be fixed - it's up to a specific implementation whether
 * subsequent calls for the same container produce the same indices or not.
 *
 * Derived classes are meant to override the @a materialise() method.
 * This method is expected to produce pairs of selected elements given the size of the collection.
 */
class PairSelection
{
public:
	PairSelection() = default;
	PairSelection(PairSelection const&) = delete;
	PairSelection& operator=(PairSelection const&) = delete;
	virtual ~PairSelection() = default;

	virtual std::vector<std::tuple<size_t, size_t>> materialise(size_t _poolSize) const = 0;
};

}
