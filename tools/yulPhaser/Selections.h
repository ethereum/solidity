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
 * Contains an abstract base class representing a selection of elements from a collection
 * and its concrete implementations.
 */

#pragma once

#include <vector>

namespace solidity::phaser
{

/**
 * Abstract base class for selections of elements from a collection.
 *
 * An instance of this class represents a specific method of selecting a set of elements from
 * containers of arbitrary sizes. The set of selected elements is always a subset of the container
 * but may indicate the same element more than once. The selection may or may not be fixed - it's
 * up to a specific implementation whether subsequent calls for the same container produce the same
 * indices or not.
 *
 * Derived classes are meant to override the @a materialise() method.
 * This method is expected to produce indices of selected elements given the size of the collection.
 */
class Selection
{
public:
	Selection() = default;
	Selection(Selection const&) = delete;
	Selection& operator=(Selection const&) = delete;
	virtual ~Selection() = default;

	virtual std::vector<size_t> materialise(size_t _poolSize) const = 0;
};

}
