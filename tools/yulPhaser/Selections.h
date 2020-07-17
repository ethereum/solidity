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
/**
 * Contains an abstract base class representing a selection of elements from a collection
 * and its concrete implementations.
 */

#pragma once

#include <cassert>
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

/**
 * A selection that selects a contiguous slice of the container. Start and end of this part are
 * specified as percentages of its size.
 */
class RangeSelection: public Selection
{
public:
	explicit RangeSelection(double _startPercent = 0.0, double _endPercent = 1.0):
		m_startPercent(_startPercent),
		m_endPercent(_endPercent)
	{
		assert(0 <= m_startPercent && m_startPercent <= m_endPercent && m_endPercent <= 1.0);
	}

	std::vector<size_t> materialise(size_t _poolSize) const override;

private:
	double m_startPercent;
	double m_endPercent;
};

/**
 * A selection that selects elements at specific, fixed positions indicated by a repeating "pattern".
 * If the positions in the pattern exceed the size of the container, they are capped at the maximum
 * available position. Always selects as many elements as the size of the container multiplied by
 * @a _selectionSize (unless the container is empty).
 *
 * E.g. if the pattern is {0, 9} and collection size is 5, the selection will materialise into
 * {0, 4, 0, 4, 0}. If the size is 3, it will be {0, 2, 0}.
 */
class MosaicSelection: public Selection
{
public:
	explicit MosaicSelection(std::vector<size_t> _pattern, double _selectionSize = 1.0):
		m_pattern(move(_pattern)),
		m_selectionSize(_selectionSize)
	{
		assert(m_pattern.size() > 0 || _selectionSize == 0.0);
	}

	std::vector<size_t> materialise(size_t _poolSize) const override;

private:
	std::vector<size_t> m_pattern;
	double m_selectionSize;
};

/**
 * A selection that randomly selects elements from a container. The resulting set of indices may
 * contain duplicates and is different on each call to @a materialise(). Always selects as many
 * elements as the size of the container multiplied by @a _selectionSize (unless the container is
 * empty).
 */
class RandomSelection: public Selection
{
public:
	explicit RandomSelection(double _selectionSize):
		m_selectionSize(_selectionSize)
	{
		assert(_selectionSize >= 0);
	}

	std::vector<size_t> materialise(size_t _poolSize) const override;

private:
	double m_selectionSize;
};

/**
 * A selection that goes over all elements in a container, for each one independently deciding
 * whether to select it or not. Each element has the same chance of being selected and can be
 * selected at most once. The order of selected elements is the same as the order of elements in
 * the container. The number of selected elements is random and can be different with each call
 * to @a materialise().
 */
class RandomSubset: public Selection
{
public:
	explicit RandomSubset(double _selectionChance):
		m_selectionChance(_selectionChance)
	{
		assert(0.0 <= _selectionChance && _selectionChance <= 1.0);
	}

	std::vector<size_t> materialise(size_t _poolSize) const override;

private:
	double m_selectionChance;
};

}
