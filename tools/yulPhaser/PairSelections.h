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

#pragma once

#include <tuple>
#include <vector>

namespace solidity::phaser
{

class PairSelection
{
public:
	PairSelection() = default;
	PairSelection(PairSelection const&) = delete;
	PairSelection& operator=(PairSelection const&) = delete;
	virtual ~PairSelection() = default;

	virtual std::vector<std::tuple<size_t, size_t>> materialize(size_t _poolSize) const = 0;
};

class RandomPairSelection: public PairSelection
{
public:
	explicit RandomPairSelection(double _selectionSize):
		m_selectionSize(_selectionSize) {}

	std::vector<std::tuple<size_t, size_t>> materialize(size_t _poolSize) const override;

private:
	double m_selectionSize;
};

}
