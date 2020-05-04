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

#include <cassert>
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

/**
 * A selection that selects pairs of random elements from a container. The resulting set of pairs
 * may contain the same pair more than once but does not contain pairs of duplicates. Always
 * selects as many pairs as the size of the container multiplied by @a _selectionSize (unless the
 * container is empty).
 */
class RandomPairSelection: public PairSelection
{
public:
	explicit RandomPairSelection(double _selectionSize):
		m_selectionSize(_selectionSize) {}

	std::vector<std::tuple<size_t, size_t>> materialise(size_t _poolSize) const override;

private:
	double m_selectionSize;
};


/**
 * A selection that goes over all elements in a container, for each one independently decides
 * whether to select it or not and then randomly combines those elements into pairs. If the number
 * of elements is odd, randomly decides whether to take one more or exclude one.
 *
 * Each element has the same chance of being selected and can be selected at most once.
 * The number of selected elements is random and can be different with each call to
 * @a materialise().
 */
class PairsFromRandomSubset: public PairSelection
{
public:
	explicit PairsFromRandomSubset(double _selectionChance):
		m_selectionChance(_selectionChance) {}

	std::vector<std::tuple<size_t, size_t>> materialise(size_t _poolSize) const override;

private:
	double m_selectionChance;
};

/**
 * A selection that selects pairs of elements at specific, fixed positions indicated by a repeating
 * "pattern". If the positions in the pattern exceed the size of the container, they are capped at
 * the maximum available position. Always selects as many pairs as the size of the container
 * multiplied by @a _selectionSize (unless the container is empty).
 *
 * E.g. if the pattern is {{0, 1}, {3, 9}} and collection size is 5, the selection will materialise
 * into {{0, 1}, {3, 4}, {0, 1}, {3, 4}, {0, 1}}. If the size is 3, it will be
 * {{0, 1}, {2, 2}, {0, 1}}.
 */
class PairMosaicSelection: public PairSelection
{
public:
	explicit PairMosaicSelection(std::vector<std::tuple<size_t, size_t>> _pattern, double _selectionSize = 1.0):
		m_pattern(move(_pattern)),
		m_selectionSize(_selectionSize)
	{
		assert(m_pattern.size() > 0 || _selectionSize == 0.0);
	}

	std::vector<std::tuple<size_t, size_t>> materialise(size_t _poolSize) const override;

private:
	std::vector<std::tuple<size_t, size_t>> m_pattern;
	double m_selectionSize;
};

}
